#include "MapClickInfo.h"

namespace carto {

    MapClickInfo::MapClickInfo(const ClickInfo& clickInfo, const MapPos& clickPos) :
        _clickInfo(clickInfo),
        _clickPos(clickPos)
    {
    }
    
    MapClickInfo::~MapClickInfo() {
    }

    ClickType::ClickType MapClickInfo::getClickType() const {
        return _clickInfo.getClickType();
    }
    
    const ClickInfo& MapClickInfo::getClickInfo() const {
        return _clickInfo;
    }
    
    const MapPos& MapClickInfo::getClickPos() const {
        return _clickPos;
    }
    
}
