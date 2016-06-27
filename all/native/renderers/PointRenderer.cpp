#include "PointRenderer.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/Texture.h"
#include "graphics/TextureManager.h"
#include "graphics/shaders/RegularShaderSource.h"
#include "graphics/ViewState.h"
#include "layers/VectorLayer.h"
#include "projections/Projection.h"
#include "renderers/drawdatas/PointDrawData.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/components/StyleTextureCache.h"
#include "utils/Const.h"
#include "utils/GLES2.h"
#include "utils/GLUtils.h"
#include "utils/Log.h"
#include "vectorelements/Point.h"

#include <cglib/mat.h>

namespace carto {

    PointRenderer::PointRenderer() :
        _elements(),
        _tempElements(),
        _drawDataBuffer(),
        _prevBitmap(nullptr),
        _colorBuf(),
        _coordBuf(),
        _indexBuf(),
        _texCoordBuf(),
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
    
    void PointRenderer::offsetLayerHorizontally(double offset) {
        // Offset current draw data batch horizontally by the required amount
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<Point>& element : _elements) {
            element->getDrawData()->offsetHorizontally(offset);
        }
    }
    
    void PointRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        _shader = shaderManager->createShader(regular_shader_source);
    
        // Get shader variables locations
        glUseProgram(_shader->getProgId());
        _a_color = _shader->getAttribLoc("a_color");
        _a_coord = _shader->getAttribLoc("a_coord");
        _a_texCoord = _shader->getAttribLoc("a_texCoord");
        _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
        _u_tex = _shader->getUniformLoc("u_tex");
    }
    
    void PointRenderer::onDrawFrame(float deltaSeconds, StyleTextureCache& styleCache, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (_elements.empty()) {
            // Early return, to avoid calling glUseProgram etc.
            return;
        }
        
        bind(viewState);
    
        // Draw points, batch by bitmap
        for (const std::shared_ptr<Point>& element : _elements) {
            std::shared_ptr<PointDrawData> drawData = element->getDrawData();
            addToBatch(drawData, styleCache, viewState);
        }
        drawBatch(styleCache, viewState);
        
        unbind();
    
        GLUtils::checkGLError("PointRenderer::onDrawFrame");
    }
    
    void PointRenderer::onSurfaceDestroyed() {
        _shader.reset();
    }
    
    void PointRenderer::addElement(const std::shared_ptr<Point>& element) {
        _tempElements.push_back(element);
    }
    
    void PointRenderer::refreshElements() {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.clear();
        _elements.swap(_tempElements);
    }
        
    void PointRenderer::updateElement(const std::shared_ptr<Point>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (std::find(_elements.begin(), _elements.end(), element) == _elements.end()) {
            _elements.push_back(element);
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
                                            StyleTextureCache& styleCache,
                                            const ViewState& viewState)
    {
        // Resize the buffers, if necessary
        if (coordBuf.size() < drawDataBuffer.size() * 4 * 3) {
            coordBuf.resize(std::min(drawDataBuffer.size() * 4 * 3, GLUtils::MAX_VERTEXBUFFER_SIZE * 3));
            texCoordBuf.resize(std::min(drawDataBuffer.size() * 4 * 2, GLUtils::MAX_VERTEXBUFFER_SIZE * 2));
            colorBuf.resize(std::min(drawDataBuffer.size() * 4 * 4, GLUtils::MAX_VERTEXBUFFER_SIZE * 4));
            indexBuf.resize(std::min(drawDataBuffer.size() * 6, GLUtils::MAX_VERTEXBUFFER_SIZE));
        }
    
        // Calculate and draw buffers
        const MapPos& cameraPos = viewState.getCameraPos();
        GLuint drawDataIndex = 0;
        for (size_t i = 0; i < drawDataBuffer.size(); i++) {
            const std::shared_ptr<PointDrawData>& drawData = drawDataBuffer[i];
            const cglib::vec3<double>& pos = drawData->getPos();
            cglib::vec3<double> translate(pos(0) - cameraPos.getX(), pos(1) - cameraPos.getY(), pos(2) - cameraPos.getZ());
    
            // Check for possible overflow in the buffers
            if ((drawDataIndex + 1) * 6 > GLUtils::MAX_VERTEXBUFFER_SIZE) {
                // If it doesn't fit, stop and draw the buffers
                glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuf[0]);
                glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, &coordBuf[0]);
                glVertexAttribPointer(a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, &texCoordBuf[0]);
                glDrawElements(GL_TRIANGLES, drawDataIndex * 6, GL_UNSIGNED_SHORT, &indexBuf[0]);
                // Start filling buffers from the beginning
                drawDataIndex = 0;
            }
    
            // Calculate coordinates
            float coordScale = drawData->getSize() * viewState.getUnitToDPCoef() * 0.5f;
            int coordIndex = drawDataIndex * 4 * 3;
            coordBuf[coordIndex + 0] = translate(0) - coordScale;
            coordBuf[coordIndex + 1] = translate(1) + coordScale;
            coordBuf[coordIndex + 2] = translate(2);
            coordBuf[coordIndex + 3] = translate(0) - coordScale;
            coordBuf[coordIndex + 4] = translate(1) - coordScale;
            coordBuf[coordIndex + 5] = translate(2);
            coordBuf[coordIndex + 6] = translate(0) + coordScale;
            coordBuf[coordIndex + 7] = translate(1) + coordScale;
            coordBuf[coordIndex + 8] = translate(2);
            coordBuf[coordIndex + 9] = translate(0) + coordScale;
            coordBuf[coordIndex + 10] = translate(1) - coordScale;
            coordBuf[coordIndex + 11] = translate(2);
    
            // Calculate texture coordinates
            int texCoordIndex = drawDataIndex * 4 * 2;
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
            int colorIndex = drawDataIndex * 4 * 4;
            for (int i = 0; i < 16; i += 4) {
                colorBuf[colorIndex + i] = color.getR();
                colorBuf[colorIndex + i + 1] = color.getG();
                colorBuf[colorIndex + i + 2] = color.getB();
                colorBuf[colorIndex + i + 3] = color.getA();
            }
    
            // Calculate indices
            int indexIndex = drawDataIndex * 6;
            int vertexIndex = drawDataIndex * 4;
            indexBuf[indexIndex + 0] = vertexIndex;
            indexBuf[indexIndex + 1] = vertexIndex + 1;
            indexBuf[indexIndex + 2] = vertexIndex + 2;
            indexBuf[indexIndex + 3] = vertexIndex + 1;
            indexBuf[indexIndex + 4] = vertexIndex + 3;
            indexBuf[indexIndex + 5] = vertexIndex + 2;
            
            drawDataIndex++;
        }
        
        // Draw the final batch
        if (drawDataIndex > 0) {
            glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuf[0]);
            glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, &coordBuf[0]);
            glVertexAttribPointer(a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, &texCoordBuf[0]);
            glDrawElements(GL_TRIANGLES, drawDataIndex * 6, GL_UNSIGNED_SHORT, &indexBuf[0]);
        }
    }
    
    bool PointRenderer::FindElementRayIntersection(const std::shared_ptr<VectorElement>& element,
                                                   const std::shared_ptr<PointDrawData>& drawData,
                                                   const std::shared_ptr<VectorLayer>& layer,
                                                   const cglib::ray3<double>& ray,
                                                   const ViewState& viewState,
                                                   std::vector<RayIntersectedElement>& results)
    {
        const cglib::vec3<double>& pos = drawData->getPos();
            
        // Calculate coordinates
        float coordScale = drawData->getSize() * viewState.getUnitToDPCoef() * 0.5f * drawData->getClickScale();
        cglib::vec3<double> topLeft    (pos(0) - coordScale, pos(1) + coordScale, pos(2));
        cglib::vec3<double> bottomLeft (pos(0) - coordScale, pos(1) - coordScale, pos(2));
        cglib::vec3<double> topRight   (pos(0) + coordScale, pos(1) + coordScale, pos(2));
        cglib::vec3<double> bottomRight(pos(0) + coordScale, pos(1) - coordScale, pos(2));
            
        // If either triangle intersects the ray, add the element to result list
        double t = 0;
        if (cglib::intersect_triangle(topLeft, bottomLeft, topRight, ray, &t) ||
            cglib::intersect_triangle(bottomLeft, bottomRight, topRight, ray, &t))
        {
            MapPos clickPos(ray(t)(0), ray(t)(1), ray(t)(2));
            const std::shared_ptr<Projection>& projection = layer->getDataSource()->getProjection();
            int priority = static_cast<int>(results.size());
            results.push_back(RayIntersectedElement(std::static_pointer_cast<VectorElement>(element), layer, projection->fromInternal(clickPos), projection->fromInternal(MapPos(pos(0), pos(1), pos(2))), priority));
            return true;
        }
        return false;
    }
    
    void PointRenderer::bind(const ViewState& viewState) {
        // Prepare for drawing
        glUseProgram(_shader->getProgId());
        // Texture
        glUniform1i(_u_tex, 0);
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
    
    void PointRenderer::addToBatch(const std::shared_ptr<PointDrawData>& drawData, StyleTextureCache& styleCache, const ViewState& viewState) {
        const Bitmap* bitmap = drawData->getBitmap().get();
        
        if (!_drawDataBuffer.empty() && ((_prevBitmap != bitmap))) {
            drawBatch(styleCache, viewState);
        }
        
        _drawDataBuffer.push_back(std::move(drawData));
        _prevBitmap = bitmap;
    }
    
    void PointRenderer::drawBatch(StyleTextureCache& styleCache, const ViewState& viewState) {
        if (_drawDataBuffer.empty()) {
            return;
        }

        // Bind texture
        const std::shared_ptr<Bitmap>& bitmap = _drawDataBuffer.front()->getBitmap();
        std::shared_ptr<Texture> texture = styleCache.get(bitmap);
        if (!texture) {
            texture = styleCache.create(bitmap, true, false);
        }
        glBindTexture(GL_TEXTURE_2D, texture->getTexId());
        
        // Draw the draw datas
        BuildAndDrawBuffers(_a_color, _a_coord, _a_texCoord, _colorBuf, _coordBuf, _indexBuf, _texCoordBuf, _drawDataBuffer,
                            texture->getTexCoordScale(), styleCache, viewState);

        _drawDataBuffer.clear();
        _prevBitmap = nullptr;
    }
    
}
