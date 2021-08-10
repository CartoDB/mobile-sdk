#include "VectorElementClickInfo.h"

namespace carto {

    VectorElementClickInfo::VectorElementClickInfo(const ClickInfo& clickInfo, const MapPos& clickPos, const MapPos& elementClickPos,
                                                   const std::shared_ptr<VectorElement>& vectorElement,
                                                   const std::shared_ptr<Layer>& layer) :
        _clickInfo(clickInfo),
        _clickPos(clickPos),
        _elementClickPos(elementClickPos),
        _vectorElement(vectorElement),
        _layer(layer)
    {
    }
    
    VectorElementClickInfo::~VectorElementClickInfo() {
    }

    ClickType::ClickType VectorElementClickInfo::getClickType() const {
        return _clickInfo.getClickType();
    }
    
    const ClickInfo& VectorElementClickInfo::getClickInfo() const {
        return _clickInfo;
    }

    const MapPos& VectorElementClickInfo::getClickPos() const {
        return _clickPos;
    }
        
    const MapPos& VectorElementClickInfo::getElementClickPos() const {
        return _elementClickPos;
    }
    
    std::shared_ptr<VectorElement> VectorElementClickInfo::getVectorElement() const {
        return _vectorElement;
    }
    
    std::shared_ptr<Layer> VectorElementClickInfo::getLayer() const {
        return _layer;
    }

}
