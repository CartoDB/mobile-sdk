#include "LabelStyle.h"

namespace carto {

    LabelStyle::LabelStyle(const Color& color,
                           float attachAnchorPointX,
                           float attachAnchorPointY,
                           bool causesOverlap,
                           bool hideIfOverlapped,
                           float horizontalOffset,
                           float verticalOffset,
                           int placementPriority,
                           bool scaleWithDPI,
                           float anchorPointX,
                           float anchorPointY,
                           bool flippable,
                           BillboardOrientation::BillboardOrientation orientationMode,
                           BillboardScaling::BillboardScaling scalingMode) :
        BillboardStyle(color,
                       attachAnchorPointX,
                       attachAnchorPointY,
                       causesOverlap,
                       hideIfOverlapped,
                       horizontalOffset,
                       verticalOffset,
                       placementPriority,
                       scaleWithDPI),
        _anchorPointX(anchorPointX),
        _anchorPointY(anchorPointY),
        _flippable(flippable),
        _orientationMode(orientationMode),
        _scalingMode(scalingMode)
    {
    }
    
    LabelStyle::~LabelStyle() {
    }
        
    float LabelStyle::getAnchorPointX() const {
        return _anchorPointX;
    }
    
    float LabelStyle::getAnchorPointY() const {
        return _anchorPointY;
    }
    
    bool LabelStyle::isFlippable() const {
        return _flippable;
    }
    
    BillboardOrientation::BillboardOrientation LabelStyle::getOrientationMode() const {
        return _orientationMode;
    }
    
    BillboardScaling::BillboardScaling LabelStyle::getScalingMode() const {
        return _scalingMode;
    }
    
}
