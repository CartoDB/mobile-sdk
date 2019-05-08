#include "MarkerDrawData.h"
#include "graphics/Bitmap.h"
#include "geometry/PointGeometry.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "styles/BillboardStyle.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vectorelements/Billboard.h"

#include <algorithm>

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
    
    std::shared_ptr<AnimationStyle> BillboardDrawData::getAnimationStyle() const {
        return _animationStyle;
    }
    
    const std::array<cglib::vec2<float>, 4>& BillboardDrawData::getCoords() const {
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
    
    int BillboardDrawData::getPlacementPriority() const {
        return _placementPriority;
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

    float BillboardDrawData::getTransition() const {
        return _transition;
    }

    void BillboardDrawData::setTransition(float transition) {
        _transition = std::max(0.0f, std::min(1.0f, transition));
    }
    
    const cglib::vec3<double>& BillboardDrawData::getPos() const {
        return _pos;
    }
        
    void BillboardDrawData::setPos(const cglib::vec3<double>& pos, const ProjectionSurface& projectionSurface) {
        MapPos internalPos = projectionSurface.calculateMapPos(pos);
        _pos = pos;
        _xAxis = cglib::vec3<float>::convert(cglib::unit(projectionSurface.calculateVector(internalPos, MapVec(1, 0, 0))));
        _yAxis = cglib::vec3<float>::convert(cglib::unit(projectionSurface.calculateVector(internalPos, MapVec(0, 1, 0))));
        _zAxis = cglib::vec3<float>::convert(cglib::unit(projectionSurface.calculateVector(internalPos, MapVec(0, 0, 1))));
    }

    const cglib::vec3<float>& BillboardDrawData::getXAxis() const {
        return _xAxis;
    }
    
    const cglib::vec3<float>& BillboardDrawData::getYAxis() const {
        return _yAxis;
    }

    const cglib::vec3<float>& BillboardDrawData::getZAxis() const {
        return _zAxis;
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
    
    void BillboardDrawData::setScreenBottomDistance(double screenBottomDistance) {
        _screenBottomDistance = screenBottomDistance;
    }
    
    const std::weak_ptr<BillboardRenderer>& BillboardDrawData::getRenderer() const {
        return _renderer;
    }
    
    void BillboardDrawData::setRenderer(const std::weak_ptr<BillboardRenderer>& renderer) {
        _renderer = renderer;
    }

    bool BillboardDrawData::isBefore(const BillboardDrawData& drawData) const {
        // First compare placement priorities
        int priorityDelta = drawData._placementPriority - _placementPriority;
        if (priorityDelta != 0) {
            return priorityDelta > 0;
        }

        // Screen bottom distance sorting
        float screenBottomDistDelta = drawData._screenBottomDistance - _screenBottomDistance;
        if (screenBottomDistDelta != 0) {
            return screenBottomDistDelta < 0;
        }

        // If equal, use the distance to the camera plane
        double cameraPlaneZoomDistDelta = drawData._cameraPlaneZoomDistance - _cameraPlaneZoomDistance;
        if (cameraPlaneZoomDistDelta != 0) {
            return cameraPlaneZoomDistDelta < 0;
        }

        return false;
    }

    float BillboardDrawData::getClickScale() const {
        return 1.0f;
    }
    
    void BillboardDrawData::offsetHorizontally(double offset) {
        _pos(0) += offset;
        setIsOffset(true);
    }
    
    BillboardDrawData::BillboardDrawData(const Billboard& billboard,
                                         const BillboardStyle& style,
                                         const Projection& projection,
                                         const ProjectionSurface& projectionSurface,
                                         const std::shared_ptr<Bitmap>& bitmap,
                                         float anchorPointX,
                                         float anchorPointY,
                                         bool flippable,
                                         BillboardOrientation::BillboardOrientation orientationMode,
                                         BillboardScaling::BillboardScaling scalingMode,
                                         float renderScale,
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
        _animationStyle(style.getAnimationStyle()),
        _coords(),
        _flippable(flippable),
        _horizontalOffset(style.getHorizontalOffset()),
        _verticalOffset(style.getVerticalOffset()),
        _genMipmaps(true),
        _orientationMode(orientationMode),
        _causesOverlap(style.isCausesOverlap()),
        _hideIfOverlapped(style.isHideIfOverlapped()),
        _overlapping(style.isHideIfOverlapped() ? true : false),
        _transition(0.0f),
        _placementPriority(style.getPlacementPriority()),
        _pos(0, 0, 0),
        _xAxis(1, 0, 0),
        _yAxis(0, 1, 0),
        _zAxis(0, 0, 1),
        _rotation(billboard.getRotation()),
        _scaleWithDPI(style.isScaleWithDPI()),
        _scalingMode(scalingMode),
        _size(size < 0 ? (bitmap ? bitmap->getWidth() / renderScale : 0) : size),
        _cameraPlaneZoomDistance(0),
        _screenBottomDistance(0),
        _renderer()
    {
        if (billboard.getGeometry()) {
            MapPos internalPos = projection.toInternal(billboard.getGeometry()->getCenterPos());
            _pos = projectionSurface.calculatePosition(internalPos);
            _xAxis = cglib::vec3<float>::convert(cglib::unit(projectionSurface.calculateVector(internalPos, MapVec(1, 0, 0))));
            _yAxis = cglib::vec3<float>::convert(cglib::unit(projectionSurface.calculateVector(internalPos, MapVec(0, 1, 0))));
            _zAxis = cglib::vec3<float>::convert(cglib::unit(projectionSurface.calculateVector(internalPos, MapVec(0, 0, 1))));
        }

        if (auto drawData = billboard.getDrawData()) {
            _transition = drawData->_transition.load();
        }

        // If size was given in meters, calculate the approximate internal size
        if (scalingMode == BillboardScaling::BILLBOARD_SCALING_WORLD_SIZE) {
            // Don't account for the projection distortion, calculate size at the equator
            _size = static_cast<float>(_size * Const::WORLD_SIZE / Const::EARTH_CIRCUMFERENCE);
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
            cglib::mat2x2<float> rotMat = cglib::rotate2_matrix(static_cast<float>(_rotation * Const::DEG_TO_RAD));
            for (int i = 0; i < 4; i++) {
                _coords[i] = cglib::transform(_coords[i], rotMat);
            }
        }
    }
    
}
