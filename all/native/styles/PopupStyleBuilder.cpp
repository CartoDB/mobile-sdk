#include "PopupStyleBuilder.h"
#include "styles/PopupStyle.h"

namespace carto {

    PopupStyleBuilder::PopupStyleBuilder() :
        BillboardStyleBuilder()
    {
        setCausesOverlap(false);
        setScaleWithDPI(false);
    }
    
    PopupStyleBuilder::~PopupStyleBuilder() {
    }
    
    std::shared_ptr<PopupStyle> PopupStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::shared_ptr<PopupStyle>(new PopupStyle(_color,
                                                          _attachAnchorPointX,
                                                          _attachAnchorPointY,
                                                          _causesOverlap,
                                                          _hideIfOverlapped,
                                                          _horizontalOffset,
                                                          _verticalOffset,
                                                          _placementPriority,
                                                          _scaleWithDPI));
    }
        
}
