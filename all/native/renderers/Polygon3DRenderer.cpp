#include "Polygon3DRenderer.h"
#include "components/Options.h"
#include "drawdatas/Polygon3DDrawData.h"
#include "layers/VectorLayer.h"
#include "graphics/Bitmap.h"
#include "graphics/ViewState.h"
#include "projections/ProjectionSurface.h"
#include "renderers/MapRenderer.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/utils/Shader.h"
#include "renderers/utils/GLResourceManager.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vectorelements/Polygon3D.h"

#include <limits>

#include <cglib/mat.h>

namespace carto {

    Polygon3DRenderer::Polygon3DRenderer() :
        _options(),
        _mapRenderer(),
        _elements(),
        _tempElements(),
        _drawDataBuffer(),
        _colorBuf(),
        _attribBuf(),
        _coordBuf(),
        _normalBuf(),
        _shader(),
        _a_color(0),
        _a_attrib(0),
        _a_coord(0),
        _a_normal(0),
        _u_ambientColor(0),
        _u_lightColor(0),
        _u_lightDir(0),
        _u_mvpMat(0),
        _mutex()
    {
    }
    
    Polygon3DRenderer::~Polygon3DRenderer() {
    }
        
    void Polygon3DRenderer::setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer) {
        std::lock_guard<std::mutex> lock(_mutex);

        _options = options;
        _mapRenderer = mapRenderer;
        _shader.reset();
    }
    
    void Polygon3DRenderer::offsetLayerHorizontally(double offset) {
        // Offset current draw data batch horizontally by the required amount
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<Polygon3D>& element : _elements) {
            element->getDrawData()->offsetHorizontally(offset);
        }
    }
    
    void Polygon3DRenderer::onDrawFrame(float deltaSeconds, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);

        // Check if options are available
        std::shared_ptr<Options> options = _options.lock();
        if (!options) {
            return;
        }

        if (_elements.empty()) {
            // Early return, to avoid calling glUseProgram etc.
            return;
        }
        
        if (!initializeRenderer()) {
            return;
        }
        
        // Enable depth test
        glDepthMask(GL_TRUE);
    
        // Prepare for drawing
        glUseProgram(_shader->getProgId());
        // Colors, coords, normals
        glEnableVertexAttribArray(_a_color);
        glEnableVertexAttribArray(_a_attrib);
        glEnableVertexAttribArray(_a_coord);
        glEnableVertexAttribArray(_a_normal);
        // Ambient light color
        const Color& ambientLightColor = options->getAmbientLightColor();
        glUniform4f(_u_ambientColor, ambientLightColor.getR() / 255.0f, ambientLightColor.getG() / 255.0f,
                    ambientLightColor.getB() / 255.0f, ambientLightColor.getA() / 255.0f);
        // Main light color
        const Color& mainLightColor = options->getMainLightColor();
        glUniform4f(_u_lightColor, mainLightColor.getR() / 255.0f, mainLightColor.getG() / 255.0f,
                    mainLightColor.getB() / 255.0f, mainLightColor.getA() / 255.0f);
        // Main light direction
        MapPos internalFocusPos = viewState.getProjectionSurface()->calculateMapPos(viewState.getFocusPos());
        cglib::vec3<float> mainLightDir = cglib::vec3<float>::convert(cglib::unit(viewState.getProjectionSurface()->calculateVector(internalFocusPos, options->getMainLightDirection())));
        glUniform3fv(_u_lightDir, 1, mainLightDir.data());
        // Matrix
        const cglib::mat4x4<float>& mvpMat = viewState.getRTEModelviewProjectionMat();
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());
    
        // Draw
        _drawDataBuffer.clear();
        for (const std::shared_ptr<Polygon3D>& element : _elements) {
            std::shared_ptr<Polygon3DDrawData> drawData = element->getDrawData();
            _drawDataBuffer.push_back(std::move(drawData));
        }
        drawBatch(viewState);
        
        // Disable depth test
        glDepthMask(GL_FALSE);
    
        // Disable bound arrays
        glDisableVertexAttribArray(_a_color);
        glDisableVertexAttribArray(_a_attrib);
        glDisableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_normal);
    
        GLContext::CheckGLError("Polygon3DRenderer::onDrawFrame");
    }
    
    void Polygon3DRenderer::addElement(const std::shared_ptr<Polygon3D>& element) {
        if (element->getDrawData()) {
            _tempElements.push_back(element);
        }
    }
    
    void Polygon3DRenderer::refreshElements() {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.clear();
        _elements.swap(_tempElements);
    }
        
    void Polygon3DRenderer::updateElement(const std::shared_ptr<Polygon3D>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (std::find(_elements.begin(), _elements.end(), element) == _elements.end()) {
            if (element->getDrawData()) {
                _elements.push_back(element);
            }
        }
    }
    
    void Polygon3DRenderer::removeElement(const std::shared_ptr<Polygon3D>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.erase(std::remove(_elements.begin(), _elements.end(), element), _elements.end());
    }
    
    void Polygon3DRenderer::calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<Polygon3D>& element : _elements) {
            const Polygon3DDrawData& drawData = *element->getDrawData();
    
            // Bounding box check
            if (!cglib::intersect_bbox(drawData.getBoundingBox(), ray)) {
                continue;
            }
    
            // Test triangles
            double closestT = std::numeric_limits<double>::infinity();
            const std::vector<cglib::vec3<double> >& coords = drawData.getCoords();
            for (std::size_t i = 0; i < coords.size(); i += 3) {
                double t = 0;
                if (cglib::intersect_triangle(coords[i + 0], coords[i + 1], coords[i + 2], ray, &t)) {
                    closestT = std::min(closestT, t);
                }
            }
            if (std::isfinite(closestT)) {
                results.push_back(RayIntersectedElement(std::static_pointer_cast<VectorElement>(element), layer, ray(closestT), ray(closestT), true));
            }
        }
    }
    
    void Polygon3DRenderer::BuildAndDrawBuffers(GLuint a_color,
                                                GLuint a_attrib,
                                                GLuint a_coord,
                                                GLuint a_normal,
                                                std::vector<unsigned char>& colorBuf,
                                                std::vector<unsigned char>& attribBuf,
                                                std::vector<float>& coordBuf,
                                                std::vector<float>& normalBuf,
                                                std::vector<std::shared_ptr<Polygon3DDrawData> >& drawDataBuffer,
                                                const ViewState& viewState)
    {
        // Calculate buffer size
        std::size_t totalCoordCount = 0;
        for (const std::shared_ptr<Polygon3DDrawData>& drawData : drawDataBuffer) {
            totalCoordCount += drawData->getCoords().size();
        }
    
        // Resize the buffers, if necessary
        if (colorBuf.size() < totalCoordCount * 4) {
            colorBuf.resize(std::min(totalCoordCount * 4, GLContext::MAX_VERTEXBUFFER_SIZE * 4));
            attribBuf.resize(std::min(totalCoordCount * 1, GLContext::MAX_VERTEXBUFFER_SIZE * 1));
            coordBuf.resize(std::min(totalCoordCount * 3, GLContext::MAX_VERTEXBUFFER_SIZE * 3));
            normalBuf.resize(std::min(totalCoordCount * 3, GLContext::MAX_VERTEXBUFFER_SIZE * 3));
        }
    
        // View state specific data
        cglib::vec3<double> cameraPos = viewState.getCameraPos();
        std::size_t coordIndex = 0;
        for (std::size_t i = 0; i < drawDataBuffer.size(); i++) {
            const std::shared_ptr<Polygon3DDrawData>& drawData = drawDataBuffer[i];
            
            const std::vector<cglib::vec3<double> >& coords = drawData->getCoords();
            if (coords.size() > GLContext::MAX_VERTEXBUFFER_SIZE) {
                Log::Error("Polygon3DRenderer::BuildAndDrawBuffers: Maximum buffer size exceeded, 3d polygon can't be drawn");
                continue;
            }
    
            // Check for possible overflow in the buffers
            if (coordIndex + coords.size() > GLContext::MAX_VERTEXBUFFER_SIZE) {
                // If it doesn't fit, stop and draw the buffers
                glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, colorBuf.data());
                glVertexAttribPointer(a_attrib, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, attribBuf.data());
                glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, coordBuf.data());
                glVertexAttribPointer(a_normal, 3, GL_FLOAT, GL_FALSE, 0, normalBuf.data());
                glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(coordIndex));
                // Start filling buffers from the beginning
                coordIndex = 0;
            }
    
            // Coords and colors
            const Color& color = drawData->getColor();
            const Color& sideColor = drawData->getSideColor();
            const std::vector<cglib::vec3<float> >& normals = drawData->getNormals();
            const std::vector<unsigned char>& attribs = drawData->getAttribs();
            std::vector<cglib::vec3<double> >::const_iterator cit = coords.begin();
            std::vector<cglib::vec3<float> >::const_iterator nit = normals.begin();
            std::vector<unsigned char>::const_iterator ait = attribs.begin();
            for (; cit != coords.end() && nit != normals.end(); ++cit, ++nit, ait++) {
                if (*ait) {
                    colorBuf[coordIndex * 4 + 0] = color.getR();
                    colorBuf[coordIndex * 4 + 1] = color.getG();
                    colorBuf[coordIndex * 4 + 2] = color.getB();
                    colorBuf[coordIndex * 4 + 3] = color.getA();
                    attribBuf[coordIndex] = 1;
                } else {
                    colorBuf[coordIndex * 4 + 0] = sideColor.getR();
                    colorBuf[coordIndex * 4 + 1] = sideColor.getG();
                    colorBuf[coordIndex * 4 + 2] = sideColor.getB();
                    colorBuf[coordIndex * 4 + 3] = sideColor.getA();
                    attribBuf[coordIndex] = 0;
                }

                const cglib::vec3<double>& coord = *cit;
                coordBuf[coordIndex * 3 + 0] = static_cast<float>(coord(0) - cameraPos(0));
                coordBuf[coordIndex * 3 + 1] = static_cast<float>(coord(1) - cameraPos(1));
                coordBuf[coordIndex * 3 + 2] = static_cast<float>(coord(2) - cameraPos(2));
                
                const cglib::vec3<float>& normal = *nit;
                normalBuf[coordIndex * 3 + 0] = normal(0);
                normalBuf[coordIndex * 3 + 1] = normal(1);
                normalBuf[coordIndex * 3 + 2] = normal(2);

                coordIndex++;
            }
        }
    
        // Draw the buffers
        if (coordIndex > 0) {
            glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, colorBuf.data());
            glVertexAttribPointer(a_attrib, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, attribBuf.data());
            glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, coordBuf.data());
            glVertexAttribPointer(a_normal, 3, GL_FLOAT, GL_FALSE, 0, normalBuf.data());
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(coordIndex));
        }
    }
        
    bool Polygon3DRenderer::initializeRenderer() {
        if (_shader && _shader->isValid()) {
            return true;
        }

        if (auto mapRenderer = _mapRenderer.lock()) {
            _shader = mapRenderer->getGLResourceManager()->create<Shader>("polygon3d", POLYGON3D_VERTEX_SHADER, POLYGON3D_FRAGMENT_SHADER);

            // Get shader variables locations
            _a_color = _shader->getAttribLoc("a_color");
            _a_attrib = _shader->getAttribLoc("a_attrib");
            _a_coord = _shader->getAttribLoc("a_coord");
            _a_normal = _shader->getAttribLoc("a_normal");
            _u_ambientColor = _shader->getUniformLoc("u_ambientColor");
            _u_lightColor = _shader->getUniformLoc("u_lightColor");
            _u_lightDir = _shader->getUniformLoc("u_lightDir");
            _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
        }

        return _shader && _shader->isValid();
    }
    
    void Polygon3DRenderer::drawBatch(const ViewState& viewState) {
        // Draw the draw datas, multiple passes may be necessary
        BuildAndDrawBuffers(_a_color, _a_attrib, _a_coord, _a_normal, _colorBuf, _attribBuf, _coordBuf, _normalBuf, _drawDataBuffer, viewState);
    }

    const std::string Polygon3DRenderer::POLYGON3D_VERTEX_SHADER = R"GLSL(
        #version 100
        attribute vec4 a_color;
        attribute vec4 a_coord;
        attribute float a_attrib;
        attribute vec3 a_normal;
        uniform vec4 u_ambientColor;
        uniform vec4 u_lightColor;
        uniform vec3 u_lightDir;
        uniform mat4 u_mvpMat;
        varying vec4 v_color;
        void main() {
            float dotProduct = max(0.0, dot(a_normal, u_lightDir));
            vec3 lighting = vec3(a_attrib, a_attrib, a_attrib) + (u_ambientColor.rgb + u_lightColor.rgb * dotProduct) * (1.0 - a_attrib);
            v_color = a_color * vec4(lighting, 1.0);
            gl_Position = u_mvpMat * a_coord;
        }
    )GLSL";

    const std::string Polygon3DRenderer::POLYGON3D_FRAGMENT_SHADER = R"GLSL(
        #version 100
        precision mediump float;
        varying lowp vec4 v_color;
        void main() {
            vec4 color = v_color;
            if (color.a == 0.0) {
                discard;
            }
            gl_FragColor = color;
        }
    )GLSL";
}
