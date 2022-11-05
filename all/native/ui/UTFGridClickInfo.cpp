#include "UTFGridClickInfo.h"

namespace carto {

    UTFGridClickInfo::UTFGridClickInfo(const ClickInfo& clickInfo, const MapPos& clickPos, const Variant& elementInfo, const std::shared_ptr<Layer>& layer) :
        _clickInfo(clickInfo),
        _clickPos(clickPos),
        _elementInfo(elementInfo),
        _layer(layer)
    {
    }
    
    UTFGridClickInfo::~UTFGridClickInfo() {
    }

    ClickType::ClickType UTFGridClickInfo::getClickType() const {
        return _clickInfo.getClickType();
    }
    
    const ClickInfo& UTFGridClickInfo::getClickInfo() const {
        return _clickInfo;
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
