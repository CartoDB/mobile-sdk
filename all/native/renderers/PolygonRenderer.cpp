#include "PolygonRenderer.h"
#include "graphics/Bitmap.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/Texture.h"
#include "graphics/TextureManager.h"
#include "graphics/shaders/RegularShaderSource.h"
#include "graphics/ViewState.h"
#include "layers/VectorLayer.h"
#include "renderers/LineRenderer.h"
#include "renderers/drawdatas/LineDrawData.h"
#include "renderers/drawdatas/PolygonDrawData.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/components/StyleTextureCache.h"
#include "projections/Projection.h"
#include "utils/Const.h"
#include "utils/GeomUtils.h"
#include "utils/GLES2.h"
#include "utils/GLUtils.h"
#include "utils/Log.h"
#include "vectorelements/Polygon.h"

#include <cglib/mat.h>

namespace carto {

    PolygonRenderer::PolygonRenderer() :
        _elements(),
        _tempElements(),
        _drawDataBuffer(),
        _lineDrawDataBuffer(),
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
    
    PolygonRenderer::~PolygonRenderer() {
    }
    
    void PolygonRenderer::offsetLayerHorizontally(double offset) {
        // Offset current draw data batch horizontally by the required amount
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<Polygon>& element : _elements) {
            element->getDrawData()->offsetHorizontally(offset);
        }
    }
    
    void PolygonRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        _shader = shaderManager->createShader(regular_shader_source);
    
        // Get shader variables locations
        glUseProgram(_shader->getProgId());
        _a_color = _shader->getAttribLoc("a_color");
        _a_coord = _shader->getAttribLoc("a_coord");
        _a_texCoord = _shader->getAttribLoc("a_texCoord");
        _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
        _u_tex = _shader->getUniformLoc("u_tex");
    }
    
    void PolygonRenderer::onDrawFrame(float deltaSeconds, StyleTextureCache& styleCache, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (_elements.empty()) {
            // Early return, to avoid calling glUseProgram etc.
            return;
        }
       
        bind(viewState);
    
        // Draw, batch polygons with the same bitmap and no line style
        for (const std::shared_ptr<Polygon>& element : _elements) {
            addToBatch(element->getDrawData(), styleCache, viewState);
        }
        drawBatch(styleCache, viewState);
        
        unbind();
    
        GLUtils::checkGLError("PolygonRenderer::onDrawFrame");
    }
    
    void PolygonRenderer::onSurfaceDestroyed() {
        _shader.reset();
    }
    
    void PolygonRenderer::addElement(const std::shared_ptr<Polygon>& element) {
        _tempElements.push_back(element);
    }
    
    void PolygonRenderer::refreshElements() {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.clear();
        _elements.swap(_tempElements);
    }
        
    void PolygonRenderer::updateElement(const std::shared_ptr<Polygon>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (std::find(_elements.begin(), _elements.end(), element) == _elements.end()) {
            _elements.push_back(element);
        }
    }
    
    void PolygonRenderer::removeElement(const std::shared_ptr<Polygon>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.erase(std::remove(_elements.begin(), _elements.end(), element), _elements.end());
    }
    
    void PolygonRenderer::calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const MapPos& rayOrig, const MapVec& rayDir, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<Polygon>& element : _elements) {
            FindElementRayIntersection(element, element->getDrawData(), layer, rayOrig, rayDir, viewState, results);
        }
    }
    
    void PolygonRenderer::BuildAndDrawBuffers(GLuint a_color,
                                              GLuint a_coord,
                                              std::vector<unsigned char>& colorBuf,
                                              std::vector<float>& coordBuf,
                                              std::vector<unsigned short>& indexBuf,
                                              std::vector<std::shared_ptr<PolygonDrawData> >& drawDataBuffer,
                                              StyleTextureCache& styleCache,
                                              const ViewState& viewState)
    {
        // Calculate buffer size
        std::size_t totalCoordCount = 0;
        std::size_t totalIndexCount = 0;
        for (const std::shared_ptr<PolygonDrawData>& drawData : drawDataBuffer) {
            for (size_t i = 0; i < drawData->getCoords().size(); i++) {
                const std::vector<MapPos>& coords = drawData->getCoords()[i];
                const std::vector<unsigned int>& indices = drawData->getIndices()[i];
                totalCoordCount += coords.size();
                totalIndexCount += indices.size();
            }
        }
    
        // Resize the buffers, if necessary
        if (coordBuf.size() < totalCoordCount * 3) {
            colorBuf.resize(std::min(totalCoordCount * 4, GLUtils::MAX_VERTEXBUFFER_SIZE * 4));
            coordBuf.resize(std::min(totalCoordCount * 3, GLUtils::MAX_VERTEXBUFFER_SIZE * 3));
        }
    
        if (indexBuf.size() < totalIndexCount) {
            indexBuf.resize(std::min(totalIndexCount, GLUtils::MAX_VERTEXBUFFER_SIZE));
        }
    
        // View state specific data
        const MapPos& cameraPos = viewState.getCameraPos();
        std::size_t colorIndex = 0;
        std::size_t coordIndex = 0;
        GLuint indexIndex = 0;
        for (const std::shared_ptr<PolygonDrawData>& drawData : drawDataBuffer) {
            // Draw data vertex info may be split into multiple buffers, draw each one
            for (size_t i = 0; i < drawData->getCoords().size(); i++) {
                
                // Check for possible overflow in the buffers
                const std::vector<MapPos>& coords = drawData->getCoords()[i];
                const std::vector<unsigned int>& indices = drawData->getIndices()[i];
                if (indices.size() > GLUtils::MAX_VERTEXBUFFER_SIZE) {
                    Log::Error("PolygonRenderer::BuildAndDrawBuffers: Maximum buffer size exceeded, polygon can't be drawn");
                    continue;
                }
                if (indexIndex + indices.size() > GLUtils::MAX_VERTEXBUFFER_SIZE) {
                    // If it doesn't fit, stop and draw the buffers
                    glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, &coordBuf[0]);
                    glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuf[0]);
                    glDrawElements(GL_TRIANGLES, indexIndex, GL_UNSIGNED_SHORT, &indexBuf[0]);
                    // Start filling buffers from the beginning
                    colorIndex = 0;
                    coordIndex = 0;
                    indexIndex = 0;
                }
                
                // Indices
                unsigned short indexOffset = static_cast<unsigned short>(coordIndex / 3); // invariant: indexOffset <= indexIndex
                for (unsigned short index : indices) {
                    indexBuf[indexIndex] = indexOffset + index;
                    indexIndex++;
                }
                
                // Colors and coords
                const Color& color = drawData->getColor();
                for (const MapPos& pos : coords) {
                    colorBuf[colorIndex + 0] = color.getR();
                    colorBuf[colorIndex + 1] = color.getG();
                    colorBuf[colorIndex + 2] = color.getB();
                    colorBuf[colorIndex + 3] = color.getA();
                    colorIndex += 4;
                    
                    coordBuf[coordIndex + 0] = pos.getX() - cameraPos.getX();
                    coordBuf[coordIndex + 1] = pos.getY() - cameraPos.getY();
                    coordBuf[coordIndex + 2] = pos.getZ() - cameraPos.getZ();
                    coordIndex += 3;
                }
            }
        }
        
        // Draw buffers
        if (indexIndex > 0) {
            glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuf[0]);
            glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, &coordBuf[0]);
            glDrawElements(GL_TRIANGLES, indexIndex, GL_UNSIGNED_SHORT, &indexBuf[0]);
        }
    }
    
    bool PolygonRenderer::FindElementRayIntersection(const std::shared_ptr<VectorElement>& element,
                                                     const std::shared_ptr<PolygonDrawData>& drawData,
                                                     const std::shared_ptr<VectorLayer>& layer,
                                                     const MapPos& rayOrig, const MapVec& rayDir,
                                                     const ViewState& viewState,
                                                     std::vector<RayIntersectedElement>& results)
    {
        // Bounding box check
        if (!GeomUtils::RayBoundingBoxIntersect(rayOrig, rayDir, drawData->getBoundingBox())) {
            return false;
        }
        
        // Test triangles
        MapPos clickPos;
        for (size_t i = 0; i < drawData->getCoords().size(); i++) {
            const std::vector<MapPos>& coords = drawData->getCoords()[i];
            const std::vector<unsigned int>& indices = drawData->getIndices()[i];
            
            for (size_t i = 0; i < indices.size(); i += 3) {
                if (GeomUtils::RayTriangleIntersect(rayOrig, rayDir, coords[indices[i]], coords[indices[i + 1]], coords[indices[i + 2]], clickPos)) {
                    double distance = GeomUtils::DistanceFromPoint(clickPos, viewState.getCameraPos());
                    MapPos projectedClickPos = layer->getDataSource()->getProjection()->fromInternal(clickPos);
                    int priority = static_cast<int>(results.size());
                    results.push_back(RayIntersectedElement(std::static_pointer_cast<VectorElement>(element), layer, projectedClickPos, projectedClickPos, priority));
                    return true;
                }
            }
        }
        return false;
    }
    
    void PolygonRenderer::bind(const ViewState &viewState) {
        // Prepare for drawing
        glUseProgram(_shader->getProgId());
        // Colors, Coords, texCoords
        glEnableVertexAttribArray(_a_color);
        glEnableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_texCoord);
        // Matrix
        const cglib::mat4x4<float>& mvpMat = viewState.getRTEModelviewProjectionMat();
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());
        // Texture
        glUniform1i(_u_tex, 0);
    }
    
    void PolygonRenderer::unbind() {
        // Disable bound arrays
        glDisableVertexAttribArray(_a_color);
        glDisableVertexAttribArray(_a_coord);
    }
    
    bool PolygonRenderer::isEmptyBatch() const {
        return _drawDataBuffer.empty();
    }
    
    void PolygonRenderer::addToBatch(const std::shared_ptr<PolygonDrawData> &drawData, StyleTextureCache &styleCache, const ViewState &viewState) {
        const Bitmap* bitmap = drawData->getBitmap().get();
        
        if (_prevBitmap && _prevBitmap != bitmap) {
            drawBatch(styleCache, viewState);
        }
        
        _drawDataBuffer.push_back(drawData);
        _prevBitmap = bitmap;
        
        if (!drawData->getLineDrawDatas().empty()) {
            // TODO: should use unbind, _lineRenderer.bind/draw/unbind, bind
            if (_prevBitmap) {
                drawBatch(styleCache, viewState);
            }
            
            // Draw the draw datas, multiple passes may be necessary
            _lineDrawDataBuffer.clear();
            for (const LineDrawData& lineDrawData : drawData->getLineDrawDatas()) {
                _lineDrawDataBuffer.push_back(&lineDrawData);
            }
            
            LineRenderer::BuildAndDrawBuffers(_a_color, _a_coord, _a_texCoord, _colorBuf, _coordBuf, _indexBuf, _texCoordBuf, _lineDrawDataBuffer, styleCache, viewState);
        }
    }
    
    void PolygonRenderer::drawBatch(StyleTextureCache& styleCache, const ViewState& viewState) {
        if (_drawDataBuffer.empty()) {
            return;
        }

        // Texture
        std::shared_ptr<Bitmap> bitmap = _drawDataBuffer.front()->getBitmap();
        std::shared_ptr<Texture> texture = styleCache.get(bitmap);
        if (!texture) {
            texture = styleCache.create(bitmap, true, true);
        }
        glBindTexture(GL_TEXTURE_2D, texture->getTexId());
        
        // Build buffers and draw
        BuildAndDrawBuffers(_a_color, _a_coord, _colorBuf, _coordBuf, _indexBuf, _drawDataBuffer, styleCache, viewState);
        
        _drawDataBuffer.clear();
        _prevBitmap = nullptr;
    }
    
}
