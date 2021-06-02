#include "BillboardRenderer.h"
#include "graphics/Bitmap.h"
#include "graphics/ViewState.h"
#include "layers/VectorLayer.h"
#include "renderers/MapRenderer.h"
#include "renderers/utils/GLResourceManager.h"
#include "renderers/utils/Shader.h"
#include "renderers/utils/Texture.h"
#include "projections/ProjectionSurface.h"
#include "renderers/drawdatas/BillboardDrawData.h"
#include "renderers/drawdatas/NMLModelDrawData.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/components/BillboardSorter.h"
#include "renderers/utils/GLResourceManager.h"
#include "renderers/utils/NMLResources.h"
#include "styles/AnimationStyle.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vectorelements/Billboard.h"
#include "vectorelements/NMLModel.h"

#include <nml/GLModel.h>
#include <nml/GLTexture.h>
#include <nml/GLResourceManager.h>
#include <nml/Package.h>

namespace carto {

    BillboardRenderer::BillboardRenderer() :
        _mapRenderer(),
        _layer(),
        _options(),
        _elements(),
        _tempElements(),
        _drawDataBuffer(),
        _nmlDrawDataBuffer(),
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
        _nmlResources(),
        _nmlModelMap(),
        _mutex()
    {
    }
    
    BillboardRenderer::~BillboardRenderer() {
    }
    
    void BillboardRenderer::setComponents(const std::weak_ptr<VectorLayer>& layer, const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        _layer = layer;
        _mapRenderer = mapRenderer;
        _options = options;
        _textureCache.reset();
        _shader.reset();
    }

    void BillboardRenderer::offsetLayerHorizontally(double offset) {
        // Offset current draw data batch horizontally by the required amount
        std::lock_guard<std::recursive_mutex> lock(_mutex);
    
        for (const std::shared_ptr<Billboard>& element : _elements) {
            if (std::shared_ptr<BillboardDrawData> drawData = element->getDrawData()) {
                drawData->offsetHorizontally(offset);
            }
        }
    }
    
    bool BillboardRenderer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        if (_elements.empty()) {
            // Skip heavy GL calls
            return false;
        }

        if (!initializeRenderer()) {
            return false;
        }
    
        // Billboards can't be rendered in layer order, they have to be sorted globally and drawn from back to front
        bool refresh = false;
        for (auto it = _elements.begin(); it != _elements.end(); ) {
            std::shared_ptr<Billboard> element = *it++;
            std::shared_ptr<BillboardDrawData> drawData = element->getDrawData();
            if (!drawData) {
                continue;
            }

            // Update animation state
            if (UpdateBillboardAnimationState(*drawData, deltaSeconds)) {
                refresh = true;
            }

            // If the element has been removed and has become invisible, remove it from the list
            if (drawData->getRenderer().lock() != shared_from_this() && drawData->getTransition() == 0.0f) {
                it = _elements.erase(--it);
                continue;
            }

            // Add the draw data to the sorter
            if (CalculateBaseBillboardDrawData(*drawData, viewState)) {
                billboardSorter.add(drawData);
            }
        }
        return refresh;
    }
    
    void BillboardRenderer::onDrawFrameSorted(float deltaSeconds,
                                              const std::vector<std::shared_ptr<BillboardDrawData> >& billboardDrawDatas,
                                              const ViewState& viewState)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        if (billboardDrawDatas.empty()) {
            // Skip heavy GL calls
            return;
        }

        if (!initializeRenderer()) {
            return;
        }
    
        // Get layer opacity
        float opacity = 0;
        if (std::shared_ptr<VectorLayer> layer = _layer.lock()) {
            opacity = layer->getOpacity();
        }

        // Gather/draw NML models
        _nmlDrawDataBuffer.clear();
        for (const std::shared_ptr<BillboardDrawData>& drawData : billboardDrawDatas) {
            if (auto nmlDrawData = std::dynamic_pointer_cast<NMLModelDrawData>(drawData)) {
                _nmlDrawDataBuffer.push_back(std::move(nmlDrawData));
            }
        }
        if (!_nmlDrawDataBuffer.empty()) {
            if (!initializeNMLRenderer()) {
                return;
            }

            // Set expected GL state
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);

            // Draw the collected batch
            drawNMLBatch(opacity, viewState);

            // Restore expected GL state
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
        }
    
        // Prepare for drawing normal billboards
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
        glActiveTexture(GL_TEXTURE0);
        
        // Draw billboards, batch by bitmap
        _drawDataBuffer.clear();
        std::shared_ptr<Bitmap> prevBitmap;
        for (const std::shared_ptr<BillboardDrawData>& drawData : billboardDrawDatas) {
            if (auto nmlDrawData = std::dynamic_pointer_cast<NMLModelDrawData>(drawData)) {
                continue;
            }

            if (std::shared_ptr<Bitmap> bitmap = drawData->getBitmap()) {
                if (prevBitmap && prevBitmap != bitmap) {
                    drawBatch(opacity, viewState);
                    _drawDataBuffer.clear();
                }
        
                _drawDataBuffer.push_back(std::move(drawData));
                prevBitmap = bitmap;
            }
        }
   
        if (prevBitmap) {
            drawBatch(opacity, viewState);
        }

        // Cleanup
        glDisableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_texCoord);
        glDisableVertexAttribArray(_a_color);

        GLContext::CheckGLError("BillboardRenderer::onDrawFrameSorted");
    }
    
    std::size_t BillboardRenderer::getElementCount() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _elements.size();
    }
        
    void BillboardRenderer::addElement(const std::shared_ptr<Billboard>& element) {
        if (std::shared_ptr<BillboardDrawData> drawData = element->getDrawData()) {
            drawData->setRenderer(shared_from_this());
            _tempElements.push_back(element);
        }
    }
    
    void BillboardRenderer::refreshElements() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _elements.clear();
        _elements.swap(_tempElements);
    }
    
    void BillboardRenderer::updateElement(const std::shared_ptr<Billboard>& element) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        std::shared_ptr<BillboardDrawData> drawData = element->getDrawData();
        if (!drawData) {
            return;
        }
        drawData->setRenderer(shared_from_this());
        if (std::find(_elements.begin(), _elements.end(), element) == _elements.end()) {
            _elements.push_back(element);
        }
    }
    
    void BillboardRenderer::removeElement(const std::shared_ptr<Billboard>& element) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (std::shared_ptr<BillboardDrawData> drawData = element->getDrawData()) {
            drawData->setRenderer(std::weak_ptr<BillboardRenderer>());
            if (!drawData->getAnimationStyle()) {
                drawData->setTransition(0.0f);
                _elements.erase(std::remove(_elements.begin(), _elements.end(), element), _elements.end());
            }
        } else {
            _elements.erase(std::remove(_elements.begin(), _elements.end(), element), _elements.end());
        }
    }
    
    void BillboardRenderer::calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        
        // Buffer for world coordinates
        std::vector<float> coordBuf(12);
        for (const std::shared_ptr<Billboard>& element : _elements) {
            std::shared_ptr<BillboardDrawData> drawData = element->getDrawData();
            if (!drawData) {
                continue;
            }

            // Don't detect clicks on overlapping billboards that are hidden or removed elements
            if (drawData->getRenderer().lock() != shared_from_this()) {
                continue;
            }
            if (drawData->getTransition() == 0.0f) {
                continue;
            }

            if (auto nmlModel = std::dynamic_pointer_cast<NMLModel>(element)) {
                calculateNMLRayIntersections(nmlModel, layer, ray, viewState, results);
                continue;
            }
    
            if (!CalculateBillboardCoords(*drawData, viewState, coordBuf, 0, drawData->getClickScale())) {
                continue;
            }
            cglib::vec3<double> originShift = viewState.getCameraPos();
            cglib::vec3<double> topLeft     = cglib::vec3<double>(coordBuf[0], coordBuf[1], coordBuf[2]) + originShift;
            cglib::vec3<double> bottomLeft  = cglib::vec3<double>(coordBuf[3], coordBuf[4], coordBuf[5]) + originShift;
            cglib::vec3<double> topRight    = cglib::vec3<double>(coordBuf[6], coordBuf[7], coordBuf[8]) + originShift;
            cglib::vec3<double> bottomRight = cglib::vec3<double>(coordBuf[9], coordBuf[10], coordBuf[11]) + originShift;
            
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
                        results.push_back(RayIntersectedElement(std::static_pointer_cast<VectorElement>(element), layer, ray(t), drawData->getPos(), true));
                    }
                }
            }
        }
    }

    bool BillboardRenderer::UpdateBillboardAnimationState(BillboardDrawData& drawData, float deltaSeconds) {
        bool refresh = false;
        bool phaseOut = !drawData.getRenderer().lock() || (drawData.isHideIfOverlapped() && drawData.isOverlapping());
        if (auto animStyle = drawData.getAnimationStyle()) {
            float transition = drawData.getTransition();
            float step = (phaseOut ? -1.0f : 1.0f);
            float duration = (phaseOut ? animStyle->getPhaseOutDuration() : animStyle->getPhaseInDuration()) / animStyle->getRelativeSpeed();
            if ((transition < 1.0f && step > 0.0f) || (transition > 0.0f && step < 0.0f)) {
                if (duration > 0.0f) {
                    drawData.setTransition(transition + step * (transition == 0.0f || transition == 1.0f ? 0.01f : deltaSeconds) / duration);
                    refresh = true;
                } else {
                    drawData.setTransition(phaseOut ? 0.0f : 1.0f);
                }
            }
        } else {
            drawData.setTransition(phaseOut ? 0.0f : 1.0f);
        }
        return refresh;
    }
        
    void BillboardRenderer::CalculateBillboardAxis(const BillboardDrawData& drawData, const ViewState& viewState, cglib::vec3<float>& xAxis, cglib::vec3<float>& yAxis) {
        const ViewState::RotationState& rotationState = viewState.getRotationState();
        switch (drawData.getOrientationMode()) {
        case BillboardOrientation::BILLBOARD_ORIENTATION_GROUND:
            xAxis = drawData.getXAxis();
            yAxis = drawData.getYAxis();
            break;
        case BillboardOrientation::BILLBOARD_ORIENTATION_FACE_CAMERA_GROUND:
            xAxis = cglib::unit(cglib::vector_product(rotationState.yAxis, drawData.getZAxis()));
            yAxis = cglib::unit(cglib::vector_product(drawData.getZAxis(), xAxis));
            break;
        case BillboardOrientation::BILLBOARD_ORIENTATION_FACE_CAMERA:
        default:
            xAxis = rotationState.xAxis;
            yAxis = rotationState.yAxis;
            break;
        }
    }
    
    bool BillboardRenderer::CalculateBillboardCoords(const BillboardDrawData& drawData, const ViewState& viewState,
                                                     std::vector<float>& coordBuf, std::size_t drawDataIndex, float sizeScale)
    {
        cglib::vec3<float> translate = cglib::vec3<float>::convert(drawData.getPos() - viewState.getCameraPos());
        if (cglib::dot_product(drawData.getZAxis(), translate) > 0) {
            return false;
        }
        
        const std::array<cglib::vec2<float>, 4>& coords = drawData.getCoords();
        for (int i = 0; i < 4; i++) {
            std::size_t coordIndex = (drawDataIndex * 4 + i) * 3;
            float x = coords[i](0);
            float y = coords[i](1);
            
            float scale = drawData.isScaleWithDPI() ? viewState.getUnitToDPCoef() : viewState.getUnitToPXCoef();
            scale *= sizeScale;

            // Calculate scaling
            switch (drawData.getScalingMode()) {
            case BillboardScaling::BILLBOARD_SCALING_WORLD_SIZE:
                x *= sizeScale;
                y *= sizeScale;
                break;
            case BillboardScaling::BILLBOARD_SCALING_SCREEN_SIZE:
                x *= scale;
                y *= scale;
                break;
            case BillboardScaling::BILLBOARD_SCALING_CONST_SCREEN_SIZE:
            default:
                float coef = static_cast<float>(scale * drawData.getCameraPlaneZoomDistance());
                x *= coef;
                y *= coef;
                break;
            }
            
            // Calculate axis
            cglib::vec3<float> xAxis, yAxis;
            CalculateBillboardAxis(drawData, viewState, xAxis, yAxis);
        
            // Build coordinates
            coordBuf[coordIndex + 0] = x * xAxis(0) + y * yAxis(0) + translate(0);
            coordBuf[coordIndex + 1] = x * xAxis(1) + y * yAxis(1) + translate(1);
            coordBuf[coordIndex + 2] = x * xAxis(2) + y * yAxis(2) + translate(2);
        }
        return true;
    }
    
    bool BillboardRenderer::CalculateBaseBillboardDrawData(BillboardDrawData& drawData, const ViewState& viewState) {
        std::shared_ptr<Billboard> baseBillboard = drawData.getBaseBillboard().lock();
        if (!baseBillboard) {
            return true;
        }
        
        const std::shared_ptr<BillboardDrawData>& baseBillboardDrawData = baseBillboard->getDrawData();
        if (!baseBillboardDrawData) {
            return false;
        }
        
        if (!CalculateBaseBillboardDrawData(*baseBillboardDrawData, viewState)) {
            return false;
        }
        
        // Billboard is attached to another billboard, calculate position before sorting
        cglib::vec3<double> baseBillboardPos = baseBillboardDrawData->getPos();
        cglib::vec3<float> baseBillboardTranslate = cglib::vec3<float>::convert(baseBillboardPos - viewState.getCameraPos());
        if (cglib::dot_product(baseBillboardDrawData->getZAxis(), baseBillboardTranslate) > 0) {
            return false;
        }

        float halfSize = baseBillboardDrawData->getSize() * 0.5f;
        cglib::vec2<float> labelAnchorVec(((drawData.getAttachAnchorPointX() - baseBillboardDrawData->getAnchorPointX()) * halfSize),
                                          ((drawData.getAttachAnchorPointY() - baseBillboardDrawData->getAnchorPointY()) / baseBillboardDrawData->getAspect() * halfSize));
        
        if (baseBillboardDrawData->getRotation() != 0) {
            labelAnchorVec = cglib::transform(labelAnchorVec, cglib::rotate2_matrix(static_cast<float>(baseBillboardDrawData->getRotation() * Const::DEG_TO_RAD)));
        }
        
        // Calculate scaling
        float scale = baseBillboardDrawData->isScaleWithDPI() ? viewState.getUnitToDPCoef() : viewState.getUnitToPXCoef();
        switch (baseBillboardDrawData->getScalingMode()) {
        case BillboardScaling::BILLBOARD_SCALING_WORLD_SIZE:
            break;
        case BillboardScaling::BILLBOARD_SCALING_SCREEN_SIZE:
            labelAnchorVec *= scale;
            break;
        case BillboardScaling::BILLBOARD_SCALING_CONST_SCREEN_SIZE:
        default:
            {
                const cglib::mat4x4<double>& mvpMat = viewState.getModelviewProjectionMat();
                double distance = baseBillboardPos(0) * mvpMat(3, 0) + baseBillboardPos(1) * mvpMat(3, 1) + baseBillboardPos(2) * mvpMat(3, 2) + mvpMat(3, 3);
                float coef = static_cast<float>(distance * viewState.get2PowZoom() / viewState.getZoom0Distance() * scale);
                labelAnchorVec *= coef;
            }
            break;
        }
        
        // Calculate orientation
        cglib::vec3<float> xAxis, yAxis;
        CalculateBillboardAxis(*baseBillboardDrawData, viewState, xAxis, yAxis);

        // Calculate delta, update position
        cglib::vec3<double> delta = cglib::vec3<double>::convert(xAxis * labelAnchorVec(0) + yAxis * labelAnchorVec(1));
        drawData.setPos(baseBillboardPos + delta);
        return true;
    }
        
    bool BillboardRenderer::CalculateNMLModelMatrix(const NMLModelDrawData& drawData, const ViewState& viewState, cglib::mat4x4<double>& modelMat, float sizeScale) {
        cglib::vec3<float> translate = cglib::vec3<float>::convert(drawData.getPos() - viewState.getCameraPos());
        if (cglib::dot_product(drawData.getZAxis(), translate) > 0) {
            return false;
        }

        float scale = drawData.isScaleWithDPI() ? viewState.getUnitToDPCoef() : viewState.getUnitToPXCoef();
        scale *= sizeScale;

        // Calculate scaling
        switch (drawData.getScalingMode()) {
        case BillboardScaling::BILLBOARD_SCALING_WORLD_SIZE:
            scale = sizeScale;
            break;
        case BillboardScaling::BILLBOARD_SCALING_SCREEN_SIZE:
            break;
        case BillboardScaling::BILLBOARD_SCALING_CONST_SCREEN_SIZE:
        default:
            scale = static_cast<float>(scale * drawData.getCameraPlaneZoomDistance());
            break;
        }
        
        // Calculate axis
        cglib::vec3<float> xAxis, yAxis;
        BillboardRenderer::CalculateBillboardAxis(drawData, viewState, xAxis, yAxis);
        cglib::vec3<float> zAxis = cglib::vector_product(xAxis, yAxis) * (1.0f / cglib::length(xAxis));

        // Calculate matrix
        modelMat = cglib::mat4x4<double>::identity();
        for (int i = 0; i < 3; i++) {
            modelMat(i, 0) = scale * xAxis(i);
            modelMat(i, 1) = scale * yAxis(i);
            modelMat(i, 2) = scale * zAxis(i);
        }

        // Multiply with local transformation
        modelMat = drawData.getLocalFrameMat() * modelMat * drawData.getLocalTransformMat();
        return true;
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
                                                float opacity,
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
        std::size_t drawDataIndex = 0;
        for (std::size_t i = 0; i < drawDataBuffer.size(); i++) {
            const std::shared_ptr<BillboardDrawData>& drawData = drawDataBuffer[i];

            if (drawData->getTransition() == 0.0f) {
                continue;
            }
            
            // Alpha value
            int alpha = std::min(256, static_cast<int>(256 * opacity * AnimationStyle::CalculateTransition(drawData->getAnimationStyle() ? drawData->getAnimationStyle()->getFadeAnimationType() : AnimationType::ANIMATION_TYPE_NONE, drawData->getTransition())));
            
            // Check for possible overflow in the buffers
            if ((drawDataIndex + 1) * 6 > GLContext::MAX_VERTEXBUFFER_SIZE) {
                // If it doesn't fit, stop and draw the buffers
                glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, coordBuf.data());
                glVertexAttribPointer(a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, texCoordBuf.data());
                glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, colorBuf.data());
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(drawDataIndex * 6), GL_UNSIGNED_SHORT, indexBuf.data());
                // Start filling buffers from the beginning
                drawDataIndex = 0;
            }
            
            // Calculate coordinates
            float relativeSize = AnimationStyle::CalculateTransition(drawData->getAnimationStyle() ? drawData->getAnimationStyle()->getSizeAnimationType() : AnimationType::ANIMATION_TYPE_NONE, drawData->getTransition());
            if (!CalculateBillboardCoords(*drawData, viewState, coordBuf, drawDataIndex, relativeSize)) {
                continue;
            }
            
            // Billboards with ground orientation (like some texts) have to be flipped to readable
            bool flip = false;
            if (drawData->isFlippable() && drawData->getOrientationMode() == BillboardOrientation::BILLBOARD_ORIENTATION_GROUND) {
                float dAngle = std::fmod(viewState.getRotation() - drawData->getRotation() + 360.0f, 360.0f);
                flip = dAngle > 90 && dAngle < 270;
            }
            
            // Calculate texture coordinates
            std::size_t texCoordIndex = drawDataIndex * 4 * 2;
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
            std::size_t colorIndex = drawDataIndex * 4 * 4;
            for (int i = 0; i < 16; i += 4) {
                colorBuf[colorIndex + i + 0] = static_cast<unsigned char>((color.getR() * alpha) >> 8);
                colorBuf[colorIndex + i + 1] = static_cast<unsigned char>((color.getG() * alpha) >> 8);
                colorBuf[colorIndex + i + 2] = static_cast<unsigned char>((color.getB() * alpha) >> 8);
                colorBuf[colorIndex + i + 3] = static_cast<unsigned char>((color.getA() * alpha) >> 8);
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
        
        glVertexAttribPointer(a_coord, 3, GL_FLOAT, GL_FALSE, 0, coordBuf.data());
        glVertexAttribPointer(a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, texCoordBuf.data());
        glVertexAttribPointer(a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, colorBuf.data());
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(drawDataIndex * 6), GL_UNSIGNED_SHORT, indexBuf.data());
    }

    bool BillboardRenderer::initializeRenderer() {
        if (_shader && _shader->isValid() && _textureCache && _textureCache->isValid()) {
            return true;
        }

        if (auto mapRenderer = _mapRenderer.lock()) {
            _textureCache = mapRenderer->getGLResourceManager()->create<BitmapTextureCache>(TEXTURE_CACHE_SIZE);

            _shader = mapRenderer->getGLResourceManager()->create<Shader>("billboard", BILLBOARD_VERTEX_SHADER, BILLBOARD_FRAGMENT_SHADER);

            // Get shader variables locations
            _a_color = _shader->getAttribLoc("a_color");
            _a_coord = _shader->getAttribLoc("a_coord");
            _a_texCoord = _shader->getAttribLoc("a_texCoord");
            _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
            _u_tex = _shader->getUniformLoc("u_tex");
        }

        return _shader && _shader->isValid() && _textureCache && _textureCache->isValid();
    }
    
    void BillboardRenderer::drawBatch(float opacity, const ViewState& viewState) {
        if (_drawDataBuffer.empty()) {
            return;
        }

        // Bind texture
        const std::shared_ptr<Bitmap>& bitmap = _drawDataBuffer.front()->getBitmap();
        std::shared_ptr<Texture> texture = _textureCache->get(bitmap);
        if (!texture) {
            texture = _textureCache->create(bitmap, _drawDataBuffer.front()->isGenMipmaps(), false);
        }
        glBindTexture(GL_TEXTURE_2D, texture->getTexId());
        
        // Draw the draw datas, multiple passes may be necessary
        BuildAndDrawBuffers(_a_color, _a_coord, _a_texCoord, _colorBuf, _coordBuf, _indexBuf, _texCoordBuf, _drawDataBuffer,
                            texture->getTexCoordScale(), opacity, viewState);
    }

    bool BillboardRenderer::initializeNMLRenderer() {
        if (_nmlResources && _nmlResources->isValid()) {
            return true;
        }

        if (auto mapRenderer = _mapRenderer.lock()) {
            _nmlResources = mapRenderer->getGLResourceManager()->create<NMLResources>();

            _nmlModelMap.clear();
        }

        return _nmlResources && _nmlResources->isValid();
    }

    void BillboardRenderer::drawNMLBatch(float opacity, const ViewState& viewState) {
        std::shared_ptr<Options> options = _options.lock();
        if (!options) {
            return;
        }

        std::shared_ptr<nml::GLResourceManager> resourceManager = _nmlResources->getResourceManager();
        if (!resourceManager) {
            return;
        }

        // Calculate lighting state
        Color optionsAmbientLightColor = options->getAmbientLightColor();
        cglib::vec4<float> ambientLightColor = cglib::vec4<float>(optionsAmbientLightColor.getR(), optionsAmbientLightColor.getG(), optionsAmbientLightColor.getB(), optionsAmbientLightColor.getA()) * (1.0f / 255.0f);
        Color optionsMainLightColor = options->getMainLightColor();
        cglib::vec4<float> mainLightColor = cglib::vec4<float>(optionsMainLightColor.getR(), optionsMainLightColor.getG(), optionsMainLightColor.getB(), optionsMainLightColor.getA()) * (1.0f / 255.0f);
        MapVec optionsMainLightDirection = options->getMainLightDirection();
        MapPos internalFocusPos = viewState.getProjectionSurface()->calculateMapPos(viewState.getFocusPos());
        cglib::vec3<float> mainLightDir = -cglib::vec3<float>::convert(cglib::unit(viewState.getProjectionSurface()->calculateVector(internalFocusPos, optionsMainLightDirection)));

        // Draw models
        cglib::mat4x4<float> projMat = cglib::mat4x4<float>::convert(viewState.getProjectionMat());
        for (const std::shared_ptr<NMLModelDrawData>& drawData : _nmlDrawDataBuffer) {
            if (drawData->getTransition() == 0.0f) {
                continue;
            }

            // Render the element
            Color color = drawData->getColor();
            float alpha = opacity * AnimationStyle::CalculateTransition(drawData->getAnimationStyle() ? drawData->getAnimationStyle()->getFadeAnimationType() : AnimationType::ANIMATION_TYPE_NONE, drawData->getTransition());
            cglib::vec4<float> modelColor = cglib::vec4<float>(color.getR(), color.getG(), color.getB(), color.getA()) * (alpha / 255.0f);
            std::shared_ptr<nml::Model> sourceModel = drawData->getSourceModel();
            std::shared_ptr<nml::GLModel> glModel = _nmlModelMap[sourceModel];
            if (!glModel) {
                glModel = std::make_shared<nml::GLModel>(*sourceModel);
                glModel->create(*resourceManager);
                _nmlModelMap[sourceModel] = glModel;
            }
    
            float relativeSize = AnimationStyle::CalculateTransition(drawData->getAnimationStyle() ? drawData->getAnimationStyle()->getSizeAnimationType() : AnimationType::ANIMATION_TYPE_NONE, drawData->getTransition());
            cglib::mat4x4<double> modelMat;
            if (!CalculateNMLModelMatrix(*drawData, viewState, modelMat, relativeSize)) {
                continue;
            }
            cglib::mat4x4<float> mvMat = cglib::mat4x4<float>::convert(viewState.getModelviewMat() * modelMat);
            nml::RenderState renderState(projMat, mvMat, modelColor, ambientLightColor, mainLightColor, mainLightDir);

            glModel->draw(*resourceManager, renderState);
        }

        // Remove stale models
        for (auto it = _nmlModelMap.begin(); it != _nmlModelMap.end(); ) {
            if (it->first.expired()) {
                it = _nmlModelMap.erase(it);
            } else {
                it++;
            }
        }

        // Dispose unused models
        resourceManager->deleteUnused();
    }

    void BillboardRenderer::calculateNMLRayIntersections(const std::shared_ptr<NMLModel>& element, const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::shared_ptr<NMLModelDrawData> drawData = std::static_pointer_cast<NMLModelDrawData>(element->getDrawData());
        if (!drawData) {
            return;
        }

        std::shared_ptr<nml::Model> sourceModel = drawData->getSourceModel();
        auto modelIt = _nmlModelMap.find(sourceModel);
        if (modelIt == _nmlModelMap.end()) {
            return;
        }
        std::shared_ptr<nml::GLModel> glModel = modelIt->second;

        cglib::mat4x4<double> modelMat;
        if (!CalculateNMLModelMatrix(*drawData, viewState, modelMat)) {
            return;
        }
        cglib::mat4x4<double> invModelMat = cglib::inverse(modelMat);

        cglib::ray3<double> rayModel = cglib::transform_ray(ray, invModelMat);
        cglib::bbox3<double> modelBounds = cglib::bbox3<double>::convert(glModel->getBounds());
        if (!cglib::intersect_bbox(modelBounds, rayModel)) {
            return;
        }

        std::vector<nml::RayIntersection> intersections;
        glModel->calculateRayIntersections(rayModel, intersections);
        
        for (std::size_t i = 0; i < intersections.size(); i++) {
            cglib::vec3<double> pos = cglib::transform_point(intersections[i].pos, modelMat);
            results.push_back(RayIntersectedElement(std::static_pointer_cast<VectorElement>(element), layer, pos, pos, true));
        }
    }

    const std::string BillboardRenderer::BILLBOARD_VERTEX_SHADER = R"GLSL(
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

    const std::string BillboardRenderer::BILLBOARD_FRAGMENT_SHADER = R"GLSL(
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

    const unsigned int BillboardRenderer::TEXTURE_CACHE_SIZE = 16 * 1024 * 1024;

}
