#include "VectorElementClickInfo.h"

namespace carto {

    VectorElementClickInfo::VectorElementClickInfo(ClickType::ClickType clickType, const MapPos& clickPos, const MapPos& elementClickPos,
                                                   const std::shared_ptr<VectorElement>& vectorElement,
                                                   const std::shared_ptr<Layer>& layer) :
        _clickType(clickType),
        _clickPos(clickPos),
        _elementClickPos(elementClickPos),
        _vectorElement(vectorElement),
        _layer(layer)
    {
    }
    
    VectorElementClickInfo::~VectorElementClickInfo() {
    }

    ClickType::ClickType VectorElementClickInfo::getClickType() const {
        return _clickType;
    }
    
    const MapPos& VectorElementClickInfo::getClickPos() const {
        return _clickPos;
    }
        
    const MapPos& VectorElementClickInfo::getElementClickPos() const {
        return _elementClickPos;
    }
    
    std::shared_ptr<VectorElement> VectorElementClickInfo::getVectorElement() const {
        return _vectorElement.get();
    }
    
    std::shared_ptr<Layer> VectorElementClickInfo::getLayer() const {
        return _layer;
    }

}
