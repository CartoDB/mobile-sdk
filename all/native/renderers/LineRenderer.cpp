#include "LineRenderer.h"
#include "graphics/Bitmap.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/Texture.h"
#include "graphics/TextureManager.h"
#include "graphics/shaders/RegularShaderSource.h"
#include "graphics/ViewState.h"
#include "layers/VectorLayer.h"
#include "projections/Projection.h"
#include "renderers/drawdatas/LineDrawData.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/components/StyleTextureCache.h"
#include "utils/Const.h"
#include "utils/GeomUtils.h"
#include "utils/GLUtils.h"
#include "utils/Log.h"
#include "vectorelements/Line.h"

#include <cglib/mat.h>
#include <cglib/vec.h>

namespace carto {
    
    LineRenderer::LineRenderer() :
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
    
    LineRenderer::~LineRenderer() {
    }
    
    void LineRenderer::offsetLayerHorizontally(double offset) {
        // Offset current draw data batch horizontally by the required amount
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<Line>& element : _elements) {
            element->getDrawData()->offsetHorizontally(offset);
        }
    }
    
    void LineRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        _shader = shaderManager->createShader(regular_shader_source);
    
        // Get shader variables locations
        glUseProgram(_shader->getProgId());
        _a_color = _shader->getAttribLoc("a_color");
        _a_coord = _shader->getAttribLoc("a_coord");
        _a_texCoord = _shader->getAttribLoc("a_texCoord");
        _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
        _u_tex = _shader->getUniformLoc("u_tex");
    }
    
    void LineRenderer::onDrawFrame(float deltaSeconds, StyleTextureCache& styleCache, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (_elements.empty()) {
            // Early return, to avoid calling glUseProgram etc.
            return;
        }
        
        bind(viewState);
    
        // Draw, batch by bitmap
        for (const std::shared_ptr<Line>& element : _elements) {
            addToBatch(element->getDrawData(), styleCache, viewState);
        }
        drawBatch(styleCache, viewState);
        
        unbind();
    
        GLUtils::checkGLError("LineRenderer::onDrawFrame");
    }
    
    void LineRenderer::onSurfaceDestroyed() {
        _shader.reset();
    }
    
    void LineRenderer::addElement(const std::shared_ptr<Line>& element) {
        _tempElements.push_back(element);
    }
    
    void LineRenderer::refreshElements() {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.clear();
        _elements.swap(_tempElements);
    }
        
    void LineRenderer::updateElement(const std::shared_ptr<Line>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (std::find(_elements.begin(), _elements.end(), element) == _elements.end()) {
            _elements.push_back(element);
        }
    }
        
    void LineRenderer::removeElement(const std::shared_ptr<Line>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.erase(std::remove(_elements.begin(), _elements.end(), element), _elements.end());
    }
    
    void LineRenderer::calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const MapPos& rayOrig, const MapVec& rayDir,
            const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        std::vector<MapPos> worldCoords;
        for (const std::shared_ptr<Line>& element : _elements) {
            FindElementRayIntersection(element, element->getDrawData(), layer, rayOrig, rayDir, viewState, results);
        }
    }
        
    void LineRenderer::BuildAndDrawBuffers(GLuint a_color,
                                           GLuint a_coord,
                                           GLuint a_texCoord,
                                           std::vector<unsigned char>& colorBuf,
                                           std::vector<float>& coordBuf,
                                           std::vector<unsigned short>& indexBuf,
                                           std::vector<float>& texCoordBuf,
                                           std::vector<const LineDrawData*>& drawDataBuffer,
                                           StyleTextureCache& styleCache,
                                           const ViewState& viewState)
    {
        // Get bitmap
        std::shared_ptr<Bitmap> bitmap = drawDataBuffer.front()->getBitmap();
        
        // Calculate buffer size
        std::size_t totalCoordCount = 0;
        std::size_t totalIndexCount = 0;
        for (const LineDrawData* drawData : drawDataBuffer) {
            for (size_t i = 0; i < drawData->getCoords().size(); i++) {
                const std::vector<MapPos*>& coords = drawData->getCoords()[i];
                const std::vector<unsigned int>& indices = drawData->getIndices()[i];
                
                totalCoordCount += coords.size();
                totalIndexCount += indices.size();
            }
        }
        
        // Resize the buffers, if necessary
        if (coordBuf.size() < totalCoordCount * 3) {
            colorBuf.resize(std::min(totalCoordCount * 4, GLUtils::MAX_VERTEXBUFFER_SIZE * 4));
            coordBuf.resize(std::min(totalCoordCount * 3, GLUtils::MAX_VERTEXBUFFER_SIZE * 3));
            texCoordBuf.resize(std::min(totalCoordCount * 2, GLUtils::MAX_VERTEXBUFFER_SIZE * 2));
        }
        
        if (indexBuf.size() < totalIndexCount) {
            indexBuf.resize(std::min(totalIndexCount, GLUtils::MAX_VERTEXBUFFER_SIZE));
        }
        
        // Calculate and draw buffers
        const MapPos& cameraPos = viewState.getCameraPos();
        std::size_t colorIndex = 0;
        std::size_t coordIndex = 0;
        std::size_t texCoordIndex = 0;
        GLuint indexIndex = 0;
        float normalScale = viewState.getUnitToDPCoef();
        float texCoordYScale = (bitmap->getHeight() > 1 ? 1.0f / viewState.getUnitToDPCoef() : 1.0f);
        for (const LineDrawData* drawData : drawDataBuffer) {
            // Draw data vertex info may be split into multiple buffers, draw each one
            for (size_t i = 0; i < drawData->getCoords().size(); i++) {
                
                // Check for possible overflow in the buffer
                const std::vector<unsigned int>& indices = drawData->getIndices()[i];
                if (indexIndex + indices.size() > GLUtils::MAX_VERTEXBUFFER_SIZE) {
                    // If it doesn't fit, stop and draw the buffers
                    glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuf[0]);
                    glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, &coordBuf[0]);
                    glVertexAttribPointer(a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, &texCoordBuf[0]);
                    glDrawElements(GL_TRIANGLES, indexIndex, GL_UNSIGNED_SHORT, &indexBuf[0]);
                    // Start filling buffers from the beginning
                    colorIndex = 0;
                    coordIndex = 0;
                    indexIndex = 0;
                    texCoordIndex = 0;
                }
                
                // Indices
                std::size_t indexOffset = coordIndex / 3;
                std::vector<unsigned int>::const_iterator iit;
                for (iit = indices.begin(); iit != indices.end(); ++iit) {
                    indexBuf[indexIndex] = indexOffset + *iit;
                    indexIndex++;
                }
                
                // Coords, tex coords and colors
                const Color& color = drawData->getColor();
                const std::vector<MapPos*>& coords = drawData->getCoords()[i];
                const std::vector<cglib::vec2<float> >& normals = drawData->getNormals()[i];
                const std::vector<cglib::vec2<float> >& texCoords = drawData->getTexCoords()[i];
                auto cit = coords.begin();
                auto nit = normals.begin();
                auto tit = texCoords.begin();
                for ( ; cit != coords.end(); ++cit, ++nit, ++tit) {
                    // Coords
                    const MapPos& pos = **cit;
                    const cglib::vec2<float>& normal = *nit;
                    coordBuf[coordIndex + 0] = pos.getX() + normal(0) * normalScale - cameraPos.getX();
                    coordBuf[coordIndex + 1] = pos.getY() + normal(1) * normalScale - cameraPos.getY();
                    coordBuf[coordIndex + 2] = pos.getZ() - cameraPos.getZ();
                    coordIndex += 3;
                    
                    // Tex coords
                    const cglib::vec2<float>& texCoord = *tit;
                    texCoordBuf[texCoordIndex + 0] = texCoord(0);
                    texCoordBuf[texCoordIndex + 1] = texCoord(1) * texCoordYScale;
                    texCoordIndex += 2;
                    
                    // Colors
                    colorBuf[colorIndex + 0] = color.getR();
                    colorBuf[colorIndex + 1] = color.getG();
                    colorBuf[colorIndex + 2] = color.getB();
                    colorBuf[colorIndex + 3] = color.getA();
                    colorIndex += 4;
                }
            }
        }
        
        // Draw the final batch
        if (indexIndex > 0) {
            glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuf[0]);
            glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, &coordBuf[0]);
            glVertexAttribPointer(a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, &texCoordBuf[0]);
            glDrawElements(GL_TRIANGLES, indexIndex, GL_UNSIGNED_SHORT, &indexBuf[0]);
        }
    }
    
    bool LineRenderer::FindElementRayIntersection(const std::shared_ptr<VectorElement>& element,
                                                  const std::shared_ptr<LineDrawData>& drawData,
                                                  const std::shared_ptr<VectorLayer>& layer,
                                                  const MapPos& rayOrig, const MapVec& rayDir,
                                                  const ViewState& viewState,
                                                  std::vector<RayIntersectedElement>& results)
    {
        std::vector<MapPos> worldCoords;

        for (size_t i = 0; i < drawData->getCoords().size(); i++) {
            // Resize the buffer for calculated world coordinates
            const std::vector<MapPos*>& coords = drawData->getCoords()[i];
            worldCoords.clear();
            worldCoords.reserve(coords.size());
            
            // Calculate world coordinates and bounding box
            MapBounds bounds;
            const std::vector<cglib::vec2<float> >& normals = drawData->getNormals()[i];
            auto cit = coords.begin();
            auto nit = normals.begin();
            for ( ; cit != coords.end() && nit != normals.end(); ++cit, ++nit) {
                const MapPos& pos = **cit;
                const cglib::vec2<float>& normal = *nit;
                MapPos worldCoord(pos.getX() + normal(0) * viewState.getUnitToDPCoef() * drawData->getClickScale(),
                                  pos.getY() + normal(1) * viewState.getUnitToDPCoef() * drawData->getClickScale(),
                                  pos.getZ());
                bounds.expandToContain(worldCoord);
                worldCoords.push_back(worldCoord);
            }
            
            // Bounding box check
            if (!GeomUtils::RayBoundingBoxIntersect(rayOrig, rayDir, bounds)) {
                continue;
            }
            
            // Click test
            MapPos clickPos;
            const std::vector<unsigned int>& indices = drawData->getIndices()[i];
            const MapPos* prevPos = nullptr;
            for (size_t i = 0; i < indices.size(); i += 3) {
                // Figure out the start and end point of the current line segment
                const MapPos* pos = prevPos;
                for (size_t j = 0; j < 3; j++) {
                    const MapPos* nextPos = coords[indices[i + j]];
                    if (nextPos != pos) {
                        prevPos = pos;
                        pos = nextPos;
                    }
                }
                
                // Test a line triangle against the click position
                if (GeomUtils::RayTriangleIntersect(rayOrig, rayDir, worldCoords[indices[i]], worldCoords[indices[i + 1]], worldCoords[indices[i + 2]], clickPos)) {
                    double distance = GeomUtils::DistanceFromPoint(clickPos, viewState.getCameraPos());
                    const MapPos& linePos = GeomUtils::CalculateNearestPointOnLineSegment(clickPos, *prevPos, *pos);
                    const std::shared_ptr<Projection>& projection = layer->getDataSource()->getProjection();
                    int priority = static_cast<int>(results.size());
                    results.push_back(RayIntersectedElement(std::static_pointer_cast<VectorElement>(element), layer, projection->fromInternal(clickPos), projection->fromInternal(linePos), priority));
                    return true;
                }
            }
        }
        return false;
    }
    
    void LineRenderer::bind(const ViewState &viewState) {
        // Prepare for drawing
        glUseProgram(_shader->getProgId());
        // Coords, texCoords, colors
        glEnableVertexAttribArray(_a_color);
        glEnableVertexAttribArray(_a_coord);
        glEnableVertexAttribArray(_a_texCoord);
        // Matrix
        const cglib::mat4x4<float>& mvpMat = viewState.getRTEModelviewProjectionMat();
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());
        // Texture
        glUniform1i(_u_tex, 0);
    }
    
    void LineRenderer::unbind() {
        // Disable bound arrays
        glDisableVertexAttribArray(_a_color);
        glDisableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_texCoord);
    }
    
    bool LineRenderer::isEmptyBatch() const {
        return _drawDataBuffer.empty();
    }
    
    void LineRenderer::addToBatch(const std::shared_ptr<LineDrawData> &drawData, StyleTextureCache &styleCache, const ViewState &viewState) {
        const Bitmap* bitmap = drawData->getBitmap().get();
        
        if (_prevBitmap && (_prevBitmap != bitmap)) {
            drawBatch(styleCache, viewState);
        }
        
        _lineDrawDataBuffer.push_back(drawData.get());
        _drawDataBuffer.push_back(std::move(drawData));
        _prevBitmap = bitmap;
    }
    
    void LineRenderer::drawBatch(StyleTextureCache& styleCache, const ViewState& viewState) {
        if (_lineDrawDataBuffer.empty()) {
            return;
        }

        // Bind texture
        const std::shared_ptr<Bitmap>& bitmap = _lineDrawDataBuffer.front()->getBitmap();
        std::shared_ptr<Texture> texture = styleCache.get(bitmap);
        if (!texture) {
            texture = styleCache.create(bitmap, true, true);
        }
        glBindTexture(GL_TEXTURE_2D, texture->getTexId());
        
        BuildAndDrawBuffers(_a_color, _a_coord, _a_texCoord, _colorBuf, _coordBuf, _indexBuf, _texCoordBuf, _lineDrawDataBuffer, styleCache, viewState);

        _lineDrawDataBuffer.clear();
        _drawDataBuffer.clear();
        _prevBitmap = nullptr;
    }
    
}
