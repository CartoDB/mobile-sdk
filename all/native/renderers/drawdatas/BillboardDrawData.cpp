#include "MarkerDrawData.h"
#include "graphics/Bitmap.h"
#include "geometry/PointGeometry.h"
#include "projections/Projection.h"
#include "styles/BillboardStyle.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vectorelements/Billboard.h"

#include <cmath>

namespace carto {

    BillboardDrawData::~BillboardDrawData() {
    }
    
    float BillboardDrawData::getAnchorPointX() const {
        return _anchorPointX;
    }
    
    float BillboardDrawData::getAnchorPointY() const {
        return _anchorPointY;
    }
    
    float BillboardDrawData::getAttachAnchorPointX() const {
        return _attachAnchorPointX;
    }
    
    float BillboardDrawData::getAttachAnchorPointY() const {
        return _attachAnchorPointY;
    }
        
    float BillboardDrawData::getAspect() const {
        return _aspect;
    }
    
    const std::weak_ptr<Billboard>& BillboardDrawData::getBillboard() const {
        return _billboard;
    }
    
    const std::weak_ptr<Billboard>& BillboardDrawData::getBaseBillboard() const {
        return _baseBillboard;
    }
    
    std::shared_ptr<Bitmap> BillboardDrawData::getBitmap() const {
        return _bitmap;
    }
    
    const cglib::vec2<float>* BillboardDrawData::getCoords() const {
        return _coords;
    }
    
    bool BillboardDrawData::isFlippable() const {
        return _flippable;
    }
    
    float BillboardDrawData::getHorizontalOffset() const {
        return _horizontalOffset;
    }
    
    float BillboardDrawData::getVerticalOffset() const {
        return _verticalOffset;
    }
        
    bool BillboardDrawData::isGenMipmaps() const {
        return _genMipmaps;
    }
    
    BillboardOrientation::BillboardOrientation BillboardDrawData::getOrientationMode() const {
        return _orientationMode;
    }
    
    bool BillboardDrawData::isCausesOverlap() const {
        return _causesOverlap;
    }
    
    bool BillboardDrawData::isHideIfOverlapped() const {
        return _hideIfOverlapped;
    }
    
    bool BillboardDrawData::isOverlapping() const {
        return _overlapping;
    }
        
    void BillboardDrawData::setOverlapping(bool overlapping) {
        _overlapping = overlapping;
    }
    
    float BillboardDrawData::getPlacementPriority() const {
        return static_cast<float>(_placementPriority);
    }
    
    const cglib::vec3<double>& BillboardDrawData::getPos() const {
        return _pos;
    }
        
    void BillboardDrawData::setPos(const cglib::vec3<double>& pos) {
        _pos = pos;
    }
    
    float BillboardDrawData::getRotation() const {
        return _rotation;
    }
        
    bool BillboardDrawData::isScaleWithDPI() const {
        return _scaleWithDPI;
    }
        
    BillboardScaling::BillboardScaling BillboardDrawData::getScalingMode() const {
        return _scalingMode;
    }
    
    float BillboardDrawData::getSize() const {
        return _size;
    }
    
    double BillboardDrawData::getCameraPlaneZoomDistance() const {
        return _cameraPlaneZoomDistance;
    }
    
    void BillboardDrawData::setCameraPlaneZoomDistance(double cameraPlaneZoomDistance) {
        _cameraPlaneZoomDistance = cameraPlaneZoomDistance;
    }
    
    int BillboardDrawData::getScreenBottomDistance() const {
        return _screenBottomDistance;
    }
    
    void BillboardDrawData::setScreenBottomDistance(int screenBottomDistance) {
        _screenBottomDistance = screenBottomDistance;
    }
    
    BillboardRenderer* BillboardDrawData::getRenderer() const {
        return _renderer;
    }
    
    void BillboardDrawData::setRenderer(BillboardRenderer& renderer) {
        _renderer = &renderer;
    }
    
    void BillboardDrawData::offsetHorizontally(double offset) {
        _pos(0) += offset;
        setIsOffset(true);
    }
    
    BillboardDrawData::BillboardDrawData(const Billboard& billboard,
                                         const BillboardStyle& style,
                                         const Projection& projection,
                                         const std::shared_ptr<Bitmap>& bitmap,
                                         float anchorPointX,
                                         float anchorPointY,
                                         bool flippable,
                                         BillboardOrientation::BillboardOrientation orientationMode,
                                         BillboardScaling::BillboardScaling scalingMode,
                                         float size) :
        VectorElementDrawData(style.getColor()),
        _anchorPointX(anchorPointX),
        _anchorPointY(anchorPointY),
        _aspect(bitmap ? static_cast<float>(bitmap->getWidth()) / bitmap->getHeight() : 1.0f),
        _attachAnchorPointX(style.getAttachAnchorPointX()),
        _attachAnchorPointY(style.getAttachAnchorPointY()),
        _billboard(std::static_pointer_cast<Billboard>(const_cast<Billboard&>(billboard).shared_from_this())),
        _baseBillboard(billboard.getBaseBillboard()),
        _bitmap(bitmap),
        _coords(),
        _flippable(flippable),
        _horizontalOffset(style.getHorizontalOffset()),
        _verticalOffset(style.getVerticalOffset()),
        _genMipmaps(true),
        _orientationMode(orientationMode),
        _causesOverlap(style.isCausesOverlap()),
        _hideIfOverlapped(style.isHideIfOverlapped()),
        _overlapping(style.isHideIfOverlapped() ? true : false),
        _placementPriority(style.getPlacementPriority()),
        _pos(),
        _rotation(billboard.getRotation()),
        _scaleWithDPI(style.isScaleWithDPI()),
        _scalingMode(scalingMode),
        _size(size < 0 ? (bitmap ? bitmap->getWidth() : 0) : size),
        _cameraPlaneZoomDistance(0),
        _screenBottomDistance(0),
        _renderer()
    {
        if (billboard.getGeometry()) {
            MapPos posInternal = projection.toInternal(billboard.getGeometry()->getCenterPos());
            _pos = cglib::vec3<double>(posInternal.getX(), posInternal.getY(), posInternal.getZ());
        }

        // If size was given in meters, calculate the approximate internal size
        if (scalingMode == BillboardScaling::BILLBOARD_SCALING_WORLD_SIZE) {
            // Don't account for the projection distortion, calculate size at the equator
            _size = static_cast<float>(projection.toInternalScale(_size));
        }
        
        float left = ((-_anchorPointX - 1.0f) * 0.5f * _size + _horizontalOffset);
        float right = left + _size;
        float bottom = ((-_anchorPointY - 1.0f) * 0.5f / _aspect * _size + _verticalOffset);
        float top = bottom + _size / _aspect;
    
        _coords[0] = cglib::vec2<float>(left, top);
        _coords[1] = cglib::vec2<float>(left, bottom);
        _coords[2] = cglib::vec2<float>(right, top);
        _coords[3] = cglib::vec2<float>(right, bottom);
    
        if (_rotation != 0.0f) {
            float sin = static_cast<float>(std::sin(_rotation * Const::DEG_TO_RAD));
            float cos = static_cast<float>(std::cos(_rotation * Const::DEG_TO_RAD));
            for (int i = 0; i < 4; i++) {
                float x = cos * _coords[i](0) - sin * _coords[i](1);
                float y = sin * _coords[i](0) + cos * _coords[i](1);
                _coords[i] = cglib::vec2<float>(x, y);
            }
        }
    }
    
}
