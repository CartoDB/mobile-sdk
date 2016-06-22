#include "BillboardRenderer.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/Texture.h"
#include "graphics/TextureManager.h"
#include "graphics/shaders/RegularShaderSource.h"
#include "graphics/ViewState.h"
#include "layers/VectorLayer.h"
#include "projections/Projection.h"
#include "renderers/drawdatas/BillboardDrawData.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/components/StyleTextureCache.h"
#include "renderers/components/BillboardSorter.h"
#include "styles/BillboardStyle.h"
#include "utils/Const.h"
#include "utils/GLES2.h"
#include "utils/GLUtils.h"
#include "utils/Log.h"
#include "vectorelements/Billboard.h"

#include <cglib/mat.h>

namespace carto {
    
    void BillboardRenderer::CalculateBillboardCoords(const BillboardDrawData& drawData, const ViewState& viewState,
                                                     std::vector<float>& coordBuf, int drawDataIndex)
    {
        cglib::vec3<double> translate(drawData.getPos()(0) - viewState.getCameraPos().getX(), drawData.getPos()(1) - viewState.getCameraPos().getY(), drawData.getPos()(2) - viewState.getCameraPos().getZ());
        
        const ViewState::RotationState& rotationState = viewState.getRotationState();
        const cglib::vec2<float>* coords = drawData.getCoords();
        for (int i = 0; i < 4; i++) {
            int coordIndex = (drawDataIndex * 4 + i) * 3;
            float x = coords[i](0);
            float y = coords[i](1);
            
            float scale = drawData.isScaleWithDPI() ? viewState.getUnitToDPCoef() : viewState.getUnitToPXCoef();
            // Calculate scaling
            switch (drawData.getScalingMode()) {
                case BillboardScaling::BILLBOARD_SCALING_WORLD_SIZE:
                    break;
                case BillboardScaling::BILLBOARD_SCALING_SCREEN_SIZE:
                    x *= scale;
                    y *= scale;
                    break;
                case BillboardScaling::BILLBOARD_SCALING_CONST_SCREEN_SIZE:
                default:
                    float coef = scale * drawData.getCameraPlaneZoomDistance();
                    x *= coef;
                    y *= coef;
                    break;
            }
            
            // Calculate orientation
            switch (drawData.getOrientationMode()) {
                case BillboardOrientation::BILLBOARD_ORIENTATION_GROUND:
                    coordBuf[coordIndex + 0] = x + translate(0);
                    coordBuf[coordIndex + 1] = y + translate(1);
                    coordBuf[coordIndex + 2] = 0 + translate(2);
                    break;
                case BillboardOrientation::BILLBOARD_ORIENTATION_FACE_CAMERA_GROUND:
                    coordBuf[coordIndex + 0] = x * rotationState._m11 + y * rotationState._sinZ + translate(0);
                    coordBuf[coordIndex + 1] = x * rotationState._m21 + y * rotationState._cosZ + translate(1);
                    coordBuf[coordIndex + 2] = x * rotationState._m31 + 0                       + translate(2);
                    break;
                case BillboardOrientation::BILLBOARD_ORIENTATION_FACE_CAMERA:
                default:
                    coordBuf[coordIndex + 0] = x * rotationState._m11 + y * rotationState._m12 + translate(0);
                    coordBuf[coordIndex + 1] = x * rotationState._m21 + y * rotationState._m22 + translate(1);
                    coordBuf[coordIndex + 2] = x * rotationState._m31 + y * rotationState._m32 + translate(2);
                    break;
            }
        }
    }
    
    BillboardRenderer::BillboardRenderer() :
        _layer(),
        _elements(),
        _tempElements(),
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
    
    BillboardRenderer::~BillboardRenderer() {
    }
    
    void BillboardRenderer::offsetLayerHorizontally(double offset) {
        // Offset current draw data batch horizontally by the required amount
        std::lock_guard<std::recursive_mutex> lock(_mutex);
    
        for (const std::shared_ptr<Billboard>& element : _elements) {
            element->getDrawData()->offsetHorizontally(offset);
        }
    }
    
    void BillboardRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        _shader = shaderManager->createShader(regular_shader_source);
    
        // Get shader variables locations
        glUseProgram(_shader->getProgId());
        _a_color = _shader->getAttribLoc("a_color");
        _a_coord = _shader->getAttribLoc("a_coord");
        _a_texCoord = _shader->getAttribLoc("a_texCoord");
        _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
        _u_tex = _shader->getUniformLoc("u_tex");
    }
    
    void BillboardRenderer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
    
        // Billboards can't be rendered in layer order, they have to be sorted globally and drawn from back to front
        for (const std::shared_ptr<Billboard>& element : _elements) {
            std::shared_ptr<BillboardDrawData> drawData = element->getDrawData();
            if (calculateBaseBillboardDrawData(drawData, viewState)) {
                billboardSorter.add(drawData);
            }
        }
    }
    
    void BillboardRenderer::onDrawFrameSorted(float deltaSeconds,
                                              const std::vector<std::shared_ptr<BillboardDrawData> >& billboardDrawDatas,
                                              StyleTextureCache& styleCache, const ViewState& viewState)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
    
        // Prepare for drawing
        glUseProgram(_shader->getProgId());
        // Coords, texCoords, colors
        glEnableVertexAttribArray(_a_coord);
        glEnableVertexAttribArray(_a_texCoord);
        glEnableVertexAttribArray(_a_color);
        //Matrix
        const cglib::mat4x4<float>& mvpMat = viewState.getRTEModelviewProjectionMat();
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());
        // Texture
        glUniform1i(_u_tex, 0);
        
        // Draw billboards, batch by bitmap
        _drawDataBuffer.clear();
        const Bitmap* prevBitmap = NULL;
        for (const std::shared_ptr<BillboardDrawData>& drawData : billboardDrawDatas) {
            const Bitmap* bitmap = drawData->getBitmap().get();
    
            if (prevBitmap && (prevBitmap != bitmap)) {
                drawBatch(styleCache, viewState);
                _drawDataBuffer.clear();
            }
    
            _drawDataBuffer.push_back(std::move(drawData));
            prevBitmap = bitmap;
        }
    
        if (prevBitmap) {
            drawBatch(styleCache, viewState);
        }
    
        glDisableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_texCoord);
        glDisableVertexAttribArray(_a_color);
    
        GLUtils::checkGLError("BillboardRenderer::onDrawFrameSorted");
    }
    
    void BillboardRenderer::onSurfaceDestroyed() {
        _shader.reset();
    }
    
    size_t BillboardRenderer::getElementCount() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _elements.size();
    }
        
    void BillboardRenderer::addElement(const std::shared_ptr<Billboard>& element) {
        element->getDrawData()->setRenderer(*this);
        _tempElements.push_back(element);
    }
    
    void BillboardRenderer::refreshElements() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _elements.clear();
        _elements.swap(_tempElements);
    }
        
    void BillboardRenderer::updateElement(const std::shared_ptr<Billboard>& element) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        element->getDrawData()->setRenderer(*this);
        if (std::find(_elements.begin(), _elements.end(), element) == _elements.end()) {
            _elements.push_back(element);
        }
    }
    
    void BillboardRenderer::removeElement(const std::shared_ptr<Billboard>& element) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _elements.erase(std::remove(_elements.begin(), _elements.end(), element), _elements.end());
    }
    
    void BillboardRenderer::setLayer(const std::shared_ptr<VectorLayer>& layer) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _layer = layer;
    }
    
    std::shared_ptr<VectorLayer> BillboardRenderer::getLayer() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _layer.lock();
    }
    
    void BillboardRenderer::calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        
        // Buffer for world coordinates
        std::vector<float> coordBuf(12);
        for (const std::shared_ptr<Billboard>& element : _elements) {
            std::shared_ptr<BillboardDrawData> drawData = element->getDrawData();

            // Don't detect clicks on overlapping billboards that are hidden
            if (drawData->isHideIfOverlapped() && drawData->isOverlapping()) {
                continue;
            }
    
            CalculateBillboardCoords(*drawData, viewState, coordBuf, 0);
            cglib::vec3<double> originShift(viewState.getCameraPos().getX(), viewState.getCameraPos().getY(), viewState.getCameraPos().getZ());
            cglib::vec3<double> topLeft = cglib::vec3<double>(coordBuf[0], coordBuf[1], coordBuf[2]) + originShift;
            cglib::vec3<double> bottomLeft = cglib::vec3<double>(coordBuf[3], coordBuf[4], coordBuf[5]) + originShift;
            cglib::vec3<double> topRight = cglib::vec3<double>(coordBuf[6], coordBuf[7], coordBuf[8]) + originShift;
            cglib::vec3<double> bottomRight = cglib::vec3<double>(coordBuf[9], coordBuf[10], coordBuf[11]) + originShift;
            
            // If either triangle intersects the ray, add the element to result list
            double t = 0;
            if (cglib::intersect_triangle(topLeft, bottomLeft, topRight, ray, &t) ||
                cglib::intersect_triangle(bottomLeft, bottomRight, topRight, ray, &t))
            {
                MapPos clickPos(ray(t)(0), ray(t)(1), ray(t)(2));
                const std::shared_ptr<Projection>& projection = layer->getDataSource()->getProjection();
                int priority = static_cast<int>(results.size());
                if (viewState.getTilt() == 90) { // if distances are equal, billboards are ordered based on 2D distance
                    priority = -drawData->getScreenBottomDistance();
                }
                cglib::vec3<double> pos = drawData->getPos();
                results.push_back(RayIntersectedElement(std::static_pointer_cast<VectorElement>(element), layer, projection->fromInternal(clickPos), projection->fromInternal(MapPos(pos(0), pos(1), pos(2))), priority));
            }
        }
    }
        
    void BillboardRenderer::BuildAndDrawBuffers(GLuint a_color,
                                                GLuint a_coord,
                                                GLuint a_texCoord,
                                                std::vector<unsigned char>& colorBuf,
                                                std::vector<float>& coordBuf,
                                                std::vector<unsigned short>& indexBuf,
                                                std::vector<float>& texCoordBuf,
                                                std::vector<std::shared_ptr<BillboardDrawData> >& drawDataBuffer,
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
        GLuint drawDataIndex = 0;
        for (size_t i = 0; i < drawDataBuffer.size(); i++) {
            const std::shared_ptr<BillboardDrawData>& drawData = drawDataBuffer[i];
            
            // Check for possible overflow in the buffers
            if ((drawDataIndex + 1) * 6 > GLUtils::MAX_VERTEXBUFFER_SIZE) {
                // If it doesn't fit, stop and draw the buffers
                glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, &coordBuf[0]);
                glVertexAttribPointer(a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, &texCoordBuf[0]);
                glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuf[0]);
                glDrawElements(GL_TRIANGLES, drawDataIndex * 6, GL_UNSIGNED_SHORT, &indexBuf[0]);
                // Start filling buffers from the beginning
                drawDataIndex = 0;
            }
            
            // Overlapping billboards should be hidden
            if (drawData->isHideIfOverlapped() && drawData->isOverlapping()) {
                continue;
            }
            
            // Calculate coordinates
            CalculateBillboardCoords(*drawData, viewState, coordBuf, drawDataIndex);
            
            // Billboards with ground orientation (like some texts) have to be flipped to readable
            bool flip = false;
            if (drawData->isFlippable() && drawData->getOrientationMode() == BillboardOrientation::BILLBOARD_ORIENTATION_GROUND) {
                float dAngle = fmod(viewState.getRotation() - drawData->getRotation() + 360, 360);
                flip = dAngle > 90 && dAngle < 270;
            }
            
            // Calculate texture coordinates
            int texCoordIndex = drawDataIndex * 4 * 2;
            if (!flip) {
                texCoordBuf[texCoordIndex + 0] = 0.0f;
                texCoordBuf[texCoordIndex + 1] = texCoordScale(1);
                texCoordBuf[texCoordIndex + 2] = 0.0f;
                texCoordBuf[texCoordIndex + 3] = 0.0f;
                texCoordBuf[texCoordIndex + 4] = texCoordScale(0);
                texCoordBuf[texCoordIndex + 5] = texCoordScale(1);
                texCoordBuf[texCoordIndex + 6] = texCoordScale(0);
                texCoordBuf[texCoordIndex + 7] = 0.0f;
            } else {
                texCoordBuf[texCoordIndex + 0] = texCoordScale(0);
                texCoordBuf[texCoordIndex + 1] = 0.0f;
                texCoordBuf[texCoordIndex + 2] = texCoordScale(0);
                texCoordBuf[texCoordIndex + 3] = texCoordScale(1);
                texCoordBuf[texCoordIndex + 4] = 0.0f;
                texCoordBuf[texCoordIndex + 5] = 0.0f;
                texCoordBuf[texCoordIndex + 6] = 0.0f;
                texCoordBuf[texCoordIndex + 7] = texCoordScale(1);
            }
            
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
        
        glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, &coordBuf[0]);
        glVertexAttribPointer(a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, &texCoordBuf[0]);
        glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuf[0]);
        glDrawElements(GL_TRIANGLES, drawDataIndex * 6, GL_UNSIGNED_SHORT, &indexBuf[0]);
    }
        
    bool BillboardRenderer::calculateBaseBillboardDrawData(const std::shared_ptr<BillboardDrawData>& drawData, const ViewState& viewState) {
        std::shared_ptr<Billboard> baseBillboard = drawData->getBaseBillboard().lock();
        if (!baseBillboard) {
            return true;
        }
        
        const std::shared_ptr<BillboardDrawData>& baseBillboardDrawData = baseBillboard->getDrawData();
        if (!baseBillboardDrawData) {
            return false;
        }
        
        if (!calculateBaseBillboardDrawData(baseBillboardDrawData, viewState)) {
            return false;
        }
        
        // Billboard is attached to another billboard, calculate position before sorting
        cglib::vec3<double> baseBillboardPos = baseBillboardDrawData->getPos();
        float halfSize = baseBillboardDrawData->getSize() * 0.5f;
        MapVec labelAnchorVec(((drawData->getAttachAnchorPointX() - baseBillboardDrawData->getAnchorPointX()) * halfSize),
                              ((drawData->getAttachAnchorPointY() - baseBillboardDrawData->getAnchorPointY())
                               / baseBillboardDrawData->getAspect() * halfSize), 0);
        
        if (baseBillboardDrawData->getRotation() != 0) {
            float sin = std::sin(baseBillboardDrawData->getRotation() * Const::DEG_TO_RAD);
            float cos = std::cos(baseBillboardDrawData->getRotation() * Const::DEG_TO_RAD);
            labelAnchorVec.rotate2D(sin, cos);
        }
        
        const ViewState::RotationState& rotationState = viewState.getRotationState();
        float x = labelAnchorVec.getX();
        float y = labelAnchorVec.getY();
        
        float scale = baseBillboardDrawData->isScaleWithDPI() ? viewState.getUnitToDPCoef() : viewState.getUnitToPXCoef();
        // Calculate scaling
        switch (baseBillboardDrawData->getScalingMode()) {
            case BillboardScaling::BILLBOARD_SCALING_WORLD_SIZE:
                break;
            case BillboardScaling::BILLBOARD_SCALING_SCREEN_SIZE:
                x *= scale;
                y *= scale;
                break;
            case BillboardScaling::BILLBOARD_SCALING_CONST_SCREEN_SIZE:
            default:
                const cglib::mat4x4<double>& mvpMat = viewState.getModelviewProjectionMat();
                double distance = baseBillboardPos(0) * mvpMat(3, 0) + baseBillboardPos(1) * mvpMat(3, 1)
                + baseBillboardPos(2) * mvpMat(3, 2) + mvpMat(3, 3);
                double coef = distance * viewState.get2PowZoom() / viewState.getZoom0Distance() * scale;
                x *= coef;
                y *= coef;
                break;
        }
        
        // Calculate orientation
        switch (baseBillboardDrawData->getOrientationMode()) {
            case BillboardOrientation::BILLBOARD_ORIENTATION_GROUND:
                labelAnchorVec.setX(x);
                labelAnchorVec.setY(y);
                break;
            case BillboardOrientation::BILLBOARD_ORIENTATION_FACE_CAMERA_GROUND:
                labelAnchorVec.setX(x * rotationState._m11 + y * rotationState._sinZ);
                labelAnchorVec.setY(x * rotationState._m21 + y * rotationState._cosZ);
                labelAnchorVec.setZ(x * rotationState._m31 + 0);
                break;
            case BillboardOrientation::BILLBOARD_ORIENTATION_FACE_CAMERA:
            default:
                labelAnchorVec.setX(x * rotationState._m11 + y * rotationState._m12);
                labelAnchorVec.setY(x * rotationState._m21 + y * rotationState._m22);
                labelAnchorVec.setZ(x * rotationState._m31 + y * rotationState._m32);
                break;
        }
        drawData->setPos(baseBillboardPos + cglib::vec3<double>(labelAnchorVec.getX(), labelAnchorVec.getY(), labelAnchorVec.getZ()));
        return true;
    }
        
    void BillboardRenderer::drawBatch(StyleTextureCache& styleCache, const ViewState& viewState) {
        // Bind texture
        const std::shared_ptr<Bitmap>& bitmap = _drawDataBuffer.front()->getBitmap();
        std::shared_ptr<Texture> texture = styleCache.get(bitmap);
        if (!texture) {
            texture = styleCache.create(bitmap, _drawDataBuffer.front()->isGenMipmaps(), false);
        }
        glBindTexture(GL_TEXTURE_2D, texture->getTexId());
        
        // Draw the draw datas, multiple passes may be necessary
        BuildAndDrawBuffers(_a_color, _a_coord, _a_texCoord, _colorBuf, _coordBuf, _indexBuf, _texCoordBuf, _drawDataBuffer,
                            texture->getTexCoordScale(), styleCache, viewState);
    }
    
}
