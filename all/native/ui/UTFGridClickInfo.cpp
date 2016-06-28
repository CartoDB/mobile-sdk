#include "UTFGridClickInfo.h"

namespace carto {

    UTFGridClickInfo::UTFGridClickInfo(ClickType::ClickType clickType, const MapPos& clickPos, const Variant& elementInfo, const std::shared_ptr<Layer>& layer) :
        _clickType(clickType),
        _clickPos(clickPos),
        _elementInfo(elementInfo),
        _layer(layer)
    {
    }
    
    UTFGridClickInfo::~UTFGridClickInfo() {
    }

    ClickType::ClickType UTFGridClickInfo::getClickType() const {
        return _clickType;
    }
    
    const MapPos& UTFGridClickInfo::getClickPos() const {
        return _clickPos;
    }
        
    const Variant& UTFGridClickInfo::getElementInfo() const {
        return _elementInfo;
    }
    
    std::shared_ptr<Layer> UTFGridClickInfo::getLayer() const {
        return _layer;
    }

}
