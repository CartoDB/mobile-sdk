#include "PopupStyle.h"

namespace carto {

    PopupStyle::PopupStyle(const Color& color,
                           float attachAnchorPointX,
                           float attachAnchorPointY,
                           bool causesOverlap,
                           bool hideIfOverlapped,
                           float horizontalOffset,
                           float verticalOffset,
                           int placementPriority,
                           bool scaleWithDPI,
                           const std::shared_ptr<AnimationStyle>& animStyle) :
        BillboardStyle(color,
                       attachAnchorPointX,
                       attachAnchorPointY,
                       causesOverlap,
                       hideIfOverlapped,
                       horizontalOffset,
                       verticalOffset,
                       placementPriority,
                       scaleWithDPI,
                       animStyle)
    {
    }
    
    PopupStyle::~PopupStyle() {
    }
    
}
