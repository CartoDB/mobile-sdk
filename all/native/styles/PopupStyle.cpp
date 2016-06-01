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
                           bool scaleWithDPI) :
        BillboardStyle(color,
                       attachAnchorPointX,
                       attachAnchorPointY,
                       causesOverlap,
                       hideIfOverlapped,
                       horizontalOffset,
                       verticalOffset,
                       placementPriority,
                       scaleWithDPI)
    {
    }
    
    PopupStyle::~PopupStyle() {
    }
    
}
