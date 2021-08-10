#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "NMLModelLODTreeClickInfo.h"

namespace carto {

    NMLModelLODTreeClickInfo::NMLModelLODTreeClickInfo(const ClickInfo& clickInfo, const MapPos& clickPos, const MapPos& elementClickPos,
                                                   const std::map<std::string, std::string>& metaData,
                                                   const std::shared_ptr<Layer>& layer) :
        _clickInfo(clickInfo),
        _clickPos(clickPos),
        _elementClickPos(elementClickPos),
        _metaData(metaData),
        _layer(layer)
    {
    }
    
    NMLModelLODTreeClickInfo::~NMLModelLODTreeClickInfo() {
    }

    ClickType::ClickType NMLModelLODTreeClickInfo::getClickType() const {
        return _clickInfo.getClickType();
    }
    
    const ClickInfo& NMLModelLODTreeClickInfo::getClickInfo() const {
        return _clickInfo;
    }
    
    const MapPos& NMLModelLODTreeClickInfo::getClickPos() const {
        return _clickPos;
    }
        
    const MapPos& NMLModelLODTreeClickInfo::getElementClickPos() const {
        return _elementClickPos;
    }
    
    const std::map<std::string, std::string>& NMLModelLODTreeClickInfo::getMetaData() const {
        return _metaData;
    }
    
    std::shared_ptr<Layer> NMLModelLODTreeClickInfo::getLayer() const {
        return _layer;
    }

}

#endif
