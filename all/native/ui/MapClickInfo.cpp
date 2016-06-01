#include "MapClickInfo.h"

namespace carto {

    MapClickInfo::MapClickInfo(ClickType::ClickType clickType, const MapPos& clickPos) :
            _clickType(clickType),
            _clickPos(clickPos)
    {
    }
    
    MapClickInfo::~MapClickInfo() {
    }

    ClickType::ClickType MapClickInfo::getClickType() const {
        return _clickType;
    }
    
    const MapPos& MapClickInfo::getClickPos() const {
        return _clickPos;
    }
    
}
