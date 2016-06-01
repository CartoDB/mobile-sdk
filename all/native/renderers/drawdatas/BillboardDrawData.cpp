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
    
    const MapVec* BillboardDrawData::getCoords() const {
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
    
    const MapPos& BillboardDrawData::getPos() const {
        return _pos;
    }
        
    void BillboardDrawData::setPos(const MapPos& pos) {
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
    
    double BillboardDrawData::getScreenBottomDistance() const {
        return _screenBottomDistance;
    }
    
    void BillboardDrawData::setScreenBottomDistance(double screenCornerDistance) {
        _screenBottomDistance = screenCornerDistance;
    }
    
    BillboardRenderer* BillboardDrawData::getRenderer() const {
        return _renderer;
    }
    
    void BillboardDrawData::setRenderer(BillboardRenderer& renderer) {
        _renderer = &renderer;
    }
    
    void BillboardDrawData::offsetHorizontally(double offset) {
        _pos.setX(_pos.getX() + offset);
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
        _pos(billboard.getGeometry() ? projection.toInternal(billboard.getGeometry()->getCenterPos()) : MapPos()),
        _rotation(billboard.getRotation()),
        _scaleWithDPI(style.isScaleWithDPI()),
        _scalingMode(scalingMode),
        _size(size < 0 ? (bitmap ? bitmap->getWidth() : 0) : size),
        _cameraPlaneZoomDistance(0),
        _screenBottomDistance(0),
        _renderer()
    {
        // If size was given in meters, calculate the approximate internal size
        if (scalingMode == BillboardScaling::BILLBOARD_SCALING_WORLD_SIZE) {
            /* //Take distortion into account, size is stretched near the poles
            MapPos scalePos;
            // If this billboard is attached to another billboard, find the root billboard
            if (!billboard.getGeometry()) {
                std::shared_ptr<Billboard> baseBillboard(_baseBillboard);
                while (baseBillboard) {
                    if (baseBillboard->getGeometry()) {
                        scalePos = baseBillboard->getGeometry()->getPos();
                        break;
                    }
                    baseBillboard = baseBillboard->getBaseBillboard();
                }
            } else {
                scalePos = billboard.getGeometry()->getPos();
            }
            // Calculate approzimate size towards north-east
            _size = projection.toInternalScale(scalePos, MapVec(0.70710678118f, 0.70710678118f), _size);*/
            
            // Don't account for the projection distortion, calculate size at the equator
            _size = static_cast<float>(projection.toInternalScale(_size));
        }
        
        double left = ((-_anchorPointX - 1.0f) * 0.5f * _size + _horizontalOffset);
        double right = left + _size;
        double bottom = ((-_anchorPointY - 1.0f) * 0.5f / _aspect * _size + _verticalOffset);
        double top = bottom + _size / _aspect;
    
        _coords[0].setX(left);
        _coords[0].setY(top);
        _coords[1].setX(left);
        _coords[1].setY(bottom);
        _coords[2].setX(right);
        _coords[2].setY(top);
        _coords[3].setX(right);
        _coords[3].setY(bottom);
    
        if (_rotation != 0.0f) {
            float sin = static_cast<float>(std::sin(_rotation * Const::DEG_TO_RAD));
            float cos = static_cast<float>(std::cos(_rotation * Const::DEG_TO_RAD));
            _coords[0].rotate2D(sin, cos);
            _coords[1].rotate2D(sin, cos);
            _coords[2].rotate2D(sin, cos);
            _coords[3].rotate2D(sin, cos);
        }
    }
    
}
