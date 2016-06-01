#include "BillboardStyle.h"

namespace carto {

    BillboardStyle::~BillboardStyle() {
    }
    
    float BillboardStyle::getAttachAnchorPointX() const {
        return _attachAnchorPointX;
    }
    
    float BillboardStyle::getAttachAnchorPointY() const {
        return _attachAnchorPointY;
    }
        
    bool BillboardStyle::isCausesOverlap() const {
        return _causesOverlap;
    }
    
    bool BillboardStyle::isHideIfOverlapped() const {
        return _hideIfOverlapped;
    }
        
    float BillboardStyle::getHorizontalOffset() const {
        return _horizontalOffset;
    }
    
    float BillboardStyle::getVerticalOffset() const {
        return _verticalOffset;
    }
    
    int BillboardStyle::getPlacementPriority() const {
        return _placementPriority;
    }
        
    bool BillboardStyle::isScaleWithDPI() const {
        return _scaleWithDPI;
    }
        
    BillboardStyle::BillboardStyle(const Color& color,
                                   float attachAnchorPointX,
                                   float attachAnchorPointY,
                                   bool causesOverlap,
                                   bool hideIfOverlapped,
                                   float horizontalOffset,
                                   float verticalOffset,
                                   int placementPriority,
                                   bool scaleWithDPI) :
        Style(color),
        _attachAnchorPointX(attachAnchorPointX),
        _attachAnchorPointY(attachAnchorPointY),
        _causesOverlap(causesOverlap),
        _hideIfOverlapped(hideIfOverlapped),
        _horizontalOffset(horizontalOffset),
        _verticalOffset(verticalOffset),
        _placementPriority(placementPriority),
        _scaleWithDPI(scaleWithDPI)
    {
    }
    
}
