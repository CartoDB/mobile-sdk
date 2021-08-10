#include "VectorTileClickInfo.h"

namespace carto {

    VectorTileClickInfo::VectorTileClickInfo(const ClickInfo& clickInfo, const MapPos& clickPos, const MapPos& featureClickPos,
                                             const std::shared_ptr<VectorTileFeature>& feature,
                                             const std::shared_ptr<Layer>& layer) :
        _clickInfo(clickInfo),
        _clickPos(clickPos),
        _featureClickPos(featureClickPos),
        _feature(feature),
        _layer(layer)
    {
    }
    
    VectorTileClickInfo::~VectorTileClickInfo() {
    }

    ClickType::ClickType VectorTileClickInfo::getClickType() const {
        return _clickInfo.getClickType();
    }
    
    const ClickInfo& VectorTileClickInfo::getClickInfo() const {
        return _clickInfo;
    }

    const MapPos& VectorTileClickInfo::getClickPos() const {
        return _clickPos;
    }
        
    const MapPos& VectorTileClickInfo::getFeatureClickPos() const {
        return _featureClickPos;
    }
    
    const MapTile& VectorTileClickInfo::getMapTile() const {
        return _feature->getMapTile();
    }

    long long VectorTileClickInfo::getFeatureId() const {
        return _feature->getId();
    }
    
    std::shared_ptr<VectorTileFeature> VectorTileClickInfo::getFeature() const {
        return _feature;
    }
    
    const std::string& VectorTileClickInfo::getFeatureLayerName() const {
        return _feature->getLayerName();
    }
    
    std::shared_ptr<Layer> VectorTileClickInfo::getLayer() const {
        return _layer;
    }

}
