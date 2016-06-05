#include "UTFGridClickInfo.h"

namespace carto {

    UTFGridClickInfo::UTFGridClickInfo(ClickType::ClickType clickType, const MapPos& clickPos, const std::map<std::string, std::string>& elementInfo, const std::shared_ptr<Layer>& layer) :
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
        
    const std::map<std::string, std::string>& UTFGridClickInfo::getElementInfo() const {
        return _elementInfo;
    }
    
    std::shared_ptr<Layer> UTFGridClickInfo::getLayer() const {
        return _layer;
    }

}
