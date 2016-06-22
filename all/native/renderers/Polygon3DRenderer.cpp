#include "Polygon3DRenderer.h"
#include "assets/DefaultPolygon3DPNG.h"
#include "components/Options.h"
#include "drawdatas/Polygon3DDrawData.h"
#include "layers/VectorLayer.h"
#include "graphics/Bitmap.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/TextureManager.h"
#include "graphics/shaders/DiffuseLightingShaderSource.h"
#include "graphics/ViewState.h"
#include "projections/Projection.h"
#include "renderers/components/RayIntersectedElement.h"
#include "utils/Const.h"
#include "utils/GLES2.h"
#include "utils/GLUtils.h"
#include "utils/Log.h"
#include "vectorelements/Polygon3D.h"

#include <cglib/mat.h>

namespace carto {

    Polygon3DRenderer::Polygon3DRenderer() :
        _polygon3DTex(),
        _elements(),
        _tempElements(),
        _drawDataBuffer(),
        _colorBuf(),
        _coordBuf(),
        _normalBuf(),
        _shader(),
        _a_color(0),
        _a_coord(0),
        _a_normal(0),
        _a_texCoord(0),
        _u_ambientColor(0),
        _u_lightColor(0),
        _u_lightDir(0),
        _u_mvpMat(0),
        _u_tex(0),
        _options(),
        _mutex()
    {
    }
    
    Polygon3DRenderer::~Polygon3DRenderer() {
    }
        
    void Polygon3DRenderer::setOptions(const std::weak_ptr<Options>& options) {
        _options = options;
    }
    
    void Polygon3DRenderer::offsetLayerHorizontally(double offset) {
        // Offset current draw data batch horizontally by the required amount
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<Polygon3D>& element : _elements) {
            element->getDrawData()->offsetHorizontally(offset);
        }
    
    }
    
    void Polygon3DRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        _shader = shaderManager->createShader(diffuse_lighting_shader_source);
    
        // Get shader variables locations
        glUseProgram(_shader->getProgId());
        _a_color = _shader->getAttribLoc("a_color");
        _a_coord = _shader->getAttribLoc("a_coord");
        _a_normal = _shader->getAttribLoc("a_normal");
        _a_texCoord = _shader->getAttribLoc("a_texCoord");
        _u_ambientColor = _shader->getUniformLoc("u_ambientColor");
        _u_lightColor = _shader->getUniformLoc("u_lightColor");
        _u_lightDir = _shader->getUniformLoc("u_lightDir");
        _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
        _u_tex = _shader->getUniformLoc("u_tex");
    
        // Create texture
        _polygon3DTex = textureManager->createTexture(GetPolygon3DBitmap(), false, false);
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
        
        // Enable depth test
        glEnable(GL_DEPTH_TEST);
    
        // Prepare for drawing
        glUseProgram(_shader->getProgId());
        // Colors, coords, normals, texCoords
        glEnableVertexAttribArray(_a_color);
        glEnableVertexAttribArray(_a_coord);
        glEnableVertexAttribArray(_a_normal);
        glDisableVertexAttribArray(_a_texCoord);
        // Ambient light color
        const Color& ambientLightColor = options->getAmbientLightColor();
        glUniform4f(_u_ambientColor, ambientLightColor.getR() / 255.0f, ambientLightColor.getG() / 255.0f,
                    ambientLightColor.getB() / 255.0f, ambientLightColor.getA() / 255.0f);
        // Main light color
        const Color& mainLightColor = options->getMainLightColor();
        glUniform4f(_u_lightColor, mainLightColor.getR() / 255.0f, mainLightColor.getG() / 255.0f,
                    mainLightColor.getB() / 255.0f, mainLightColor.getA() / 255.0f);
        // Main light direction
        const MapVec& mainLightDir = options->getMainLightDirection();
        glUniform3f(_u_lightDir, mainLightDir.getX(), mainLightDir.getY(), mainLightDir.getZ());
        // Matrix
        const cglib::mat4x4<float>& mvpMat = viewState.getRTEModelviewProjectionMat();
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());
        // Texture
        glUniform1i(_u_tex, 0);
        glBindTexture(GL_TEXTURE_2D, _polygon3DTex->getTexId());
    
        // Draw
        _drawDataBuffer.clear();
        for (const std::shared_ptr<Polygon3D>& element : _elements) {
            std::shared_ptr<Polygon3DDrawData> drawData = element->getDrawData();
            _drawDataBuffer.push_back(std::move(drawData));
        }
        drawBatch(viewState);
        
        // Disable depth test
        glDisable(GL_DEPTH_TEST);
    
        // Disable bound arrays
        glDisableVertexAttribArray(_a_color);
        glDisableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_normal);
    
        GLUtils::checkGLError("Polygon3DRenderer::onDrawFrame");
    }
    
    void Polygon3DRenderer::onSurfaceDestroyed() {
        _polygon3DTex.reset();
        _shader.reset();
    }
    
    void Polygon3DRenderer::addElement(const std::shared_ptr<Polygon3D>& element) {
        _tempElements.push_back(element);
    }
    
    void Polygon3DRenderer::refreshElements() {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.clear();
        _elements.swap(_tempElements);
    }
        
    void Polygon3DRenderer::updateElement(const std::shared_ptr<Polygon3D>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (std::find(_elements.begin(), _elements.end(), element) == _elements.end()) {
            _elements.push_back(element);
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
            const std::vector<cglib::vec3<double> >& coords = drawData.getCoords();
            for (size_t i = 0; i < coords.size(); i += 3) {
                double t = 0;
                if (cglib::intersect_triangle(coords[i + 0], coords[i + 1], coords[i + 2], ray, &t)) {
                    MapPos clickPos(ray(t)(0), ray(t)(1), ray(t)(2));
                    MapPos projectedClickPos = layer->getDataSource()->getProjection()->fromInternal(clickPos);
                    int priority = static_cast<int>(results.size());
                    results.push_back(RayIntersectedElement(std::static_pointer_cast<VectorElement>(element), layer, projectedClickPos, projectedClickPos, priority));
                    break;
                }
            }
        }
    }
    
    void Polygon3DRenderer::BuildAndDrawBuffers(GLuint a_color,
                                                GLuint a_coord,
                                                GLuint a_normal,
                                                std::vector<unsigned char>& colorBuf,
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
            colorBuf.resize(std::min(totalCoordCount * 4, GLUtils::MAX_VERTEXBUFFER_SIZE * 4));
            coordBuf.resize(std::min(totalCoordCount * 3, GLUtils::MAX_VERTEXBUFFER_SIZE * 3));
            normalBuf.resize(std::min(totalCoordCount * 3, GLUtils::MAX_VERTEXBUFFER_SIZE * 3));
        }
    
        // View state specific data
        const MapPos& cameraPos = viewState.getCameraPos();
        std::size_t colorIndex = 0;
        std::size_t normalIndex = 0;
        GLuint coordIndex = 0;
        for (size_t i = 0; i < drawDataBuffer.size(); i++) {
            const std::shared_ptr<Polygon3DDrawData>& drawData = drawDataBuffer[i];
            
            const std::vector<cglib::vec3<double> >& coords = drawData->getCoords();
            if (coords.size() > GLUtils::MAX_VERTEXBUFFER_SIZE) {
                Log::Error("Polygon3DRenderer::BuildAndDrawBuffers: Maximum buffer size exceeded, 3d polygon can't be drawn");
                continue;
            }
    
            // Check for possible overflow in the buffers
            if (coordIndex / 3 + coords.size() > GLUtils::MAX_VERTEXBUFFER_SIZE) {
                // If it doesn't fit, stop and draw the buffers
                glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuf[0]);
                glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, &coordBuf[0]);
                glVertexAttribPointer(a_normal, 3, GL_FLOAT, GL_FALSE, 0, &normalBuf[0]);
                glDrawArrays(GL_TRIANGLES, 0, coordIndex / 3);
                // Start filling buffers from the beginning
                colorIndex = 0;
                coordIndex = 0;
                normalIndex = 0;
            }
    
            // Coords and colors
            const Color& color = drawData->getColor();
            const Color& sideColor = drawData->getSideColor();
            const std::vector<cglib::vec3<float> >& normals = drawData->getNormals();
            std::vector<cglib::vec3<double> >::const_iterator cit;
            std::vector<cglib::vec3<float> >::const_iterator nit;
            for (cit = coords.begin(), nit = normals.begin(); cit != coords.end() && nit != normals.end(); ++cit, ++nit) {
                const cglib::vec3<double>& coord = *cit;
                coordBuf[coordIndex + 0] = coord(0) - cameraPos.getX();
                coordBuf[coordIndex + 1] = coord(1) - cameraPos.getY();
                coordBuf[coordIndex + 2] = coord(2) - cameraPos.getZ();
                coordIndex += 3;
                
                const cglib::vec3<float>& normal = *nit;
                normalBuf[normalIndex + 0] = normal(0);
                normalBuf[normalIndex + 1] = normal(1);
                normalBuf[normalIndex + 2] = normal(2);
                normalIndex += 3;

                if (normal(2) == 1) {
                    colorBuf[colorIndex + 0] = color.getR();
                    colorBuf[colorIndex + 1] = color.getG();
                    colorBuf[colorIndex + 2] = color.getB();
                    colorBuf[colorIndex + 3] = color.getA();
                } else {
                    colorBuf[colorIndex + 0] = sideColor.getR();
                    colorBuf[colorIndex + 1] = sideColor.getG();
                    colorBuf[colorIndex + 2] = sideColor.getB();
                    colorBuf[colorIndex + 3] = sideColor.getA();
                }
                colorIndex += 4;
            }
        }
    
        // Draw the buffers
        if (coordIndex > 0) {
            glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuf[0]);
            glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, &coordBuf[0]);
            glVertexAttribPointer(a_normal, 3, GL_FLOAT, GL_FALSE, 0, &normalBuf[0]);
            glDrawArrays(GL_TRIANGLES, 0, coordIndex / 3);
        }
    }
        
    std::shared_ptr<Bitmap> Polygon3DRenderer::GetPolygon3DBitmap() {
        if (!_Polygon3DBitmap) {
            _Polygon3DBitmap = Bitmap::CreateFromCompressed(default_polygon_3d_png, default_polygon_3d_png_len);
        }
        return _Polygon3DBitmap;
    }
    
    std::shared_ptr<Bitmap> Polygon3DRenderer::_Polygon3DBitmap;
    
    void Polygon3DRenderer::drawBatch(const ViewState& viewState) {
        // Draw the draw datas, multiple passes may be necessary
        BuildAndDrawBuffers(_a_color, _a_coord, _a_normal, _colorBuf, _coordBuf, _normalBuf, _drawDataBuffer, viewState);
    }
        
}
