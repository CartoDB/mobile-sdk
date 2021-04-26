#include "PointRenderer.h"
#include "graphics/Bitmap.h"
#include "graphics/ViewState.h"
#include "layers/VectorLayer.h"
#include "renderers/MapRenderer.h"
#include "renderers/drawdatas/PointDrawData.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/utils/GLResourceManager.h"
#include "renderers/utils/Shader.h"
#include "renderers/utils/Texture.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vectorelements/Point.h"

#include <cglib/mat.h>

namespace carto {

    PointRenderer::PointRenderer() :
        _mapRenderer(),
        _elements(),
        _tempElements(),
        _drawDataBuffer(),
        _prevBitmap(nullptr),
        _colorBuf(),
        _coordBuf(),
        _indexBuf(),
        _texCoordBuf(),
        _textureCache(),
        _shader(),
        _a_color(0),
        _a_coord(0),
        _a_texCoord(0),
        _u_mvpMat(0),
        _u_tex(0),
        _mutex()
    {
    }
    
    PointRenderer::~PointRenderer() {
    }
    
    void PointRenderer::setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer) {
        std::lock_guard<std::mutex> lock(_mutex);

        _mapRenderer = mapRenderer;
        _textureCache.reset();
        _shader.reset();
    }
    
    void PointRenderer::offsetLayerHorizontally(double offset) {
        // Offset current draw data batch horizontally by the required amount
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<Point>& element : _elements) {
            element->getDrawData()->offsetHorizontally(offset);
        }
    }
    
    void PointRenderer::onDrawFrame(float deltaSeconds, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (_elements.empty()) {
            // Early return, to avoid calling glUseProgram etc.
            return;
        }

        if (!initializeRenderer()) {
            return;
        }
        
        bind(viewState);
    
        // Draw points, batch by bitmap
        for (const std::shared_ptr<Point>& element : _elements) {
            std::shared_ptr<PointDrawData> drawData = element->getDrawData();
            addToBatch(drawData, viewState);
        }
        drawBatch(viewState);
        
        unbind();
    
        GLContext::CheckGLError("PointRenderer::onDrawFrame");
    }
    
    void PointRenderer::addElement(const std::shared_ptr<Point>& element) {
        if (element->getDrawData()) {
            _tempElements.push_back(element);
        }
    }
    
    void PointRenderer::refreshElements() {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.clear();
        _elements.swap(_tempElements);
    }
        
    void PointRenderer::updateElement(const std::shared_ptr<Point>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (std::find(_elements.begin(), _elements.end(), element) == _elements.end()) {
            if (element->getDrawData()) {
                _elements.push_back(element);
            }
        }
    }
    
    void PointRenderer::removeElement(const std::shared_ptr<Point>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.erase(std::remove(_elements.begin(), _elements.end(), element), _elements.end());
    }
    
    void PointRenderer::calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<Point>& element : _elements) {
            FindElementRayIntersection(element, element->getDrawData(), layer, ray, viewState, results);
        }
    }
    
    void PointRenderer::BuildAndDrawBuffers(GLuint a_color,
                                            GLuint a_coord,
                                            GLuint a_texCoord,
                                            std::vector<unsigned char>& colorBuf,
                                            std::vector<float>& coordBuf,
                                            std::vector<unsigned short>& indexBuf,
                                            std::vector<float>& texCoordBuf,
                                            std::vector<std::shared_ptr<PointDrawData> >& drawDataBuffer,
                                            const cglib::vec2<float>& texCoordScale,
                                            const ViewState& viewState)
    {
        // Resize the buffers, if necessary
        if (coordBuf.size() < drawDataBuffer.size() * 4 * 3) {
            coordBuf.resize(std::min(drawDataBuffer.size() * 4 * 3, GLContext::MAX_VERTEXBUFFER_SIZE * 3));
            texCoordBuf.resize(std::min(drawDataBuffer.size() * 4 * 2, GLContext::MAX_VERTEXBUFFER_SIZE * 2));
            colorBuf.resize(std::min(drawDataBuffer.size() * 4 * 4, GLContext::MAX_VERTEXBUFFER_SIZE * 4));
            indexBuf.resize(std::min(drawDataBuffer.size() * 6, GLContext::MAX_VERTEXBUFFER_SIZE));
        }
    
        // Calculate and draw buffers
        cglib::vec3<double> cameraPos = viewState.getCameraPos();
        std::size_t drawDataIndex = 0;
        for (std::size_t i = 0; i < drawDataBuffer.size(); i++) {
            const std::shared_ptr<PointDrawData>& drawData = drawDataBuffer[i];
            
            float coordScale = drawData->getSize() * viewState.getUnitToDPCoef() * 0.5f;
            cglib::vec3<float> translate = cglib::vec3<float>::convert(drawData->getPos() - cameraPos);
            cglib::vec3<float> dx = drawData->getXAxis() * coordScale;
            cglib::vec3<float> dy = drawData->getYAxis() * coordScale;

            // Check for possible overflow in the buffers
            if ((drawDataIndex + 1) * 6 > GLContext::MAX_VERTEXBUFFER_SIZE) {
                // If it doesn't fit, stop and draw the buffers
                glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, colorBuf.data());
                glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, coordBuf.data());
                glVertexAttribPointer(a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, texCoordBuf.data());
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(drawDataIndex * 6), GL_UNSIGNED_SHORT, indexBuf.data());
                // Start filling buffers from the beginning
                drawDataIndex = 0;
            }
    
            // Calculate coordinates
            std::size_t coordIndex = drawDataIndex * 4 * 3;
            coordBuf[coordIndex + 0] = translate(0) - dx(0) + dy(0);
            coordBuf[coordIndex + 1] = translate(1) - dx(1) + dy(1);
            coordBuf[coordIndex + 2] = translate(2) - dx(2) + dy(2);
            coordBuf[coordIndex + 3] = translate(0) - dx(0) - dy(0);
            coordBuf[coordIndex + 4] = translate(1) - dx(1) - dy(1);
            coordBuf[coordIndex + 5] = translate(2) - dx(2) - dy(2);
            coordBuf[coordIndex + 6] = translate(0) + dx(0) + dy(0);
            coordBuf[coordIndex + 7] = translate(1) + dx(1) + dy(1);
            coordBuf[coordIndex + 8] = translate(2) + dx(2) + dy(2);
            coordBuf[coordIndex + 9] = translate(0) + dx(0) - dy(0);
            coordBuf[coordIndex + 10] = translate(1) + dx(1) - dy(1);
            coordBuf[coordIndex + 11] = translate(2) + dx(2) - dy(2);
    
            // Calculate texture coordinates
            std::size_t texCoordIndex = drawDataIndex * 4 * 2;
            texCoordBuf[texCoordIndex + 0] = 0.0f;
            texCoordBuf[texCoordIndex + 1] = texCoordScale(1);
            texCoordBuf[texCoordIndex + 2] = 0.0f;
            texCoordBuf[texCoordIndex + 3] = 0.0f;
            texCoordBuf[texCoordIndex + 4] = texCoordScale(0);
            texCoordBuf[texCoordIndex + 5] = texCoordScale(1);
            texCoordBuf[texCoordIndex + 6] = texCoordScale(0);
            texCoordBuf[texCoordIndex + 7] = 0.0f;
    
            // Calculate colors
            const Color& color = drawData->getColor();
            std::size_t colorIndex = drawDataIndex * 4 * 4;
            for (int i = 0; i < 16; i += 4) {
                colorBuf[colorIndex + i + 0] = color.getR();
                colorBuf[colorIndex + i + 1] = color.getG();
                colorBuf[colorIndex + i + 2] = color.getB();
                colorBuf[colorIndex + i + 3] = color.getA();
            }
    
            // Calculate indices
            std::size_t indexIndex = drawDataIndex * 6;
            unsigned short vertexIndex = static_cast<unsigned short>(drawDataIndex * 4);
            indexBuf[indexIndex + 0] = vertexIndex + 0;
            indexBuf[indexIndex + 1] = vertexIndex + 1;
            indexBuf[indexIndex + 2] = vertexIndex + 2;
            indexBuf[indexIndex + 3] = vertexIndex + 1;
            indexBuf[indexIndex + 4] = vertexIndex + 3;
            indexBuf[indexIndex + 5] = vertexIndex + 2;
            
            drawDataIndex++;
        }
        
        // Draw the final batch
        if (drawDataIndex > 0) {
            glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, colorBuf.data());
            glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, coordBuf.data());
            glVertexAttribPointer(a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, texCoordBuf.data());
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(drawDataIndex * 6), GL_UNSIGNED_SHORT, indexBuf.data());
        }
    }
    
    bool PointRenderer::FindElementRayIntersection(const std::shared_ptr<VectorElement>& element,
                                                   const std::shared_ptr<PointDrawData>& drawData,
                                                   const std::shared_ptr<VectorLayer>& layer,
                                                   const cglib::ray3<double>& ray,
                                                   const ViewState& viewState,
                                                   std::vector<RayIntersectedElement>& results)
    {
        float coordScale = drawData->getSize() * viewState.getUnitToDPCoef() * drawData->getClickScale() * 0.5f;
        cglib::vec3<double> pos = drawData->getPos();
        cglib::vec3<double> dx = cglib::vec3<double>::convert(drawData->getXAxis() * coordScale);
        cglib::vec3<double> dy = cglib::vec3<double>::convert(drawData->getYAxis() * coordScale);
            
        // Calculate coordinates
        cglib::vec3<double> topLeft = pos - dx + dy;
        cglib::vec3<double> bottomLeft = pos - dx - dy;
        cglib::vec3<double> topRight = pos + dx + dy;
        cglib::vec3<double> bottomRight = pos + dx - dy;
            
        // Check if either triangle intersects the ray
        double t = 0;
        if (cglib::intersect_triangle(topLeft, bottomLeft, topRight, ray, &t) ||
            cglib::intersect_triangle(bottomLeft, bottomRight, topRight, ray, &t))
        {
            const std::shared_ptr<Bitmap>& bitmap = drawData->getBitmap();

            // Check that the pixel we have hit is not transparent
            cglib::vec3<double> delta = ray(t) - topLeft;
            int x = static_cast<int>(cglib::dot_product(delta, topRight - topLeft) / cglib::norm(topRight - topLeft) * bitmap->getWidth());
            int y = static_cast<int>(cglib::dot_product(delta, bottomLeft - topLeft) / cglib::norm(bottomLeft - topLeft) * bitmap->getHeight());

            if (x >= 0 && y >= 0 && x < static_cast<int>(bitmap->getWidth()) && y < static_cast<int>(bitmap->getHeight())) {
                int width  = std::min(2, static_cast<int>(bitmap->getWidth())  - x);
                int height = std::min(2, static_cast<int>(bitmap->getHeight()) - y);
                std::shared_ptr<Bitmap> subBitmap = bitmap->getSubBitmap(x, y, width, height)->getRGBABitmap();
                const std::vector<unsigned char>& pixelData = subBitmap->getPixelData();
                if (std::any_of(pixelData.begin(), pixelData.end(), [](unsigned char c) { return c != 0; })) {
                    results.push_back(RayIntersectedElement(std::static_pointer_cast<VectorElement>(element), layer, ray(t), pos, layer->isZBuffering()));
                    return true;
                }
            }
        }
        return false;
    }

    bool PointRenderer::initializeRenderer() {
        if (_shader && _shader->isValid() && _textureCache && _textureCache->isValid()) {
            return true;
        }

        if (auto mapRenderer = _mapRenderer.lock()) {
            _textureCache = mapRenderer->getGLResourceManager()->create<BitmapTextureCache>(TEXTURE_CACHE_SIZE);

            _shader = mapRenderer->getGLResourceManager()->create<Shader>("point", POINT_VERTEX_SHADER, POINT_FRAGMENT_SHADER);
    
            // Get shader variables locations
            _a_color = _shader->getAttribLoc("a_color");
            _a_coord = _shader->getAttribLoc("a_coord");
            _a_texCoord = _shader->getAttribLoc("a_texCoord");
            _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
            _u_tex = _shader->getUniformLoc("u_tex");
       }

       return _shader && _shader->isValid() && _textureCache && _textureCache->isValid();
    }
    
    void PointRenderer::bind(const ViewState& viewState) {
        // Prepare for drawing
        glUseProgram(_shader->getProgId());
        // Texture
        glUniform1i(_u_tex, 0);
        glActiveTexture(GL_TEXTURE0);
        // Matrix
        const cglib::mat4x4<float>& mvpMat = viewState.getRTEModelviewProjectionMat();
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());
        // Coords, texCoords, colors
        glEnableVertexAttribArray(_a_coord);
        glEnableVertexAttribArray(_a_texCoord);
        glEnableVertexAttribArray(_a_color);
    }
    
    void PointRenderer::unbind() {
        // Disable bound arrays
        glDisableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_texCoord);
        glDisableVertexAttribArray(_a_color);
    }
    
    bool PointRenderer::isEmptyBatch() const {
        return _drawDataBuffer.empty();
    }
    
    void PointRenderer::addToBatch(const std::shared_ptr<PointDrawData>& drawData, const ViewState& viewState) {
        const Bitmap* bitmap = drawData->getBitmap().get();
        
        if (!_drawDataBuffer.empty() && ((_prevBitmap != bitmap))) {
            drawBatch(viewState);
        }
        
        _drawDataBuffer.push_back(std::move(drawData));
        _prevBitmap = bitmap;
    }
    
    void PointRenderer::drawBatch(const ViewState& viewState) {
        if (_drawDataBuffer.empty()) {
            return;
        }

        // Bind texture
        const std::shared_ptr<Bitmap>& bitmap = _drawDataBuffer.front()->getBitmap();
        std::shared_ptr<Texture> texture = _textureCache->get(bitmap);
        if (!texture) {
            texture = _textureCache->create(bitmap, true, false);
        }
        glBindTexture(GL_TEXTURE_2D, texture->getTexId());
        
        // Draw the draw datas
        BuildAndDrawBuffers(_a_color, _a_coord, _a_texCoord, _colorBuf, _coordBuf, _indexBuf, _texCoordBuf, _drawDataBuffer,
                            texture->getTexCoordScale(), viewState);

        _drawDataBuffer.clear();
        _prevBitmap = nullptr;
    }
    
    const std::string PointRenderer::POINT_VERTEX_SHADER = R"GLSL(
        #version 100
        attribute vec4 a_coord;
        attribute vec2 a_texCoord;
        attribute vec4 a_color;
        varying vec2 v_texCoord;
        varying vec4 v_color;
        uniform mat4 u_mvpMat;
        void main() {
            v_texCoord = a_texCoord;
            v_color = a_color;
            gl_Position = u_mvpMat * a_coord;
        }
    )GLSL";

    const std::string PointRenderer::POINT_FRAGMENT_SHADER = R"GLSL(
        #version 100
        precision mediump float;
        varying mediump vec2 v_texCoord;
        varying lowp vec4 v_color;
        uniform sampler2D u_tex;
        void main() {
            vec4 color = texture2D(u_tex, v_texCoord) * v_color;
            if (color.a == 0.0) {
                discard;
            }
            gl_FragColor = color;
        }
    )GLSL";

    const unsigned int PointRenderer::TEXTURE_CACHE_SIZE = 8 * 1024 * 1024;

}
