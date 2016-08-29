#include "VectorTileClickInfo.h"

namespace carto {

    VectorTileClickInfo::VectorTileClickInfo(ClickType::ClickType clickType, const MapPos& clickPos, const MapPos& featureClickPos, const MapTile& mapTile,
                                                   const std::shared_ptr<Feature>& feature,
                                                   const std::shared_ptr<Layer>& layer) :
        _clickType(clickType),
        _clickPos(clickPos),
        _mapTile(mapTile),
        _featureClickPos(featureClickPos),
        _feature(feature),
        _layer(layer)
    {
    }
    
    VectorTileClickInfo::~VectorTileClickInfo() {
    }

    ClickType::ClickType VectorTileClickInfo::getClickType() const {
        return _clickType;
    }
    
    const MapPos& VectorTileClickInfo::getClickPos() const {
        return _clickPos;
    }
        
    const MapPos& VectorTileClickInfo::getFeatureClickPos() const {
        return _featureClickPos;
    }
    
    const MapTile& VectorTileClickInfo::getMapTile() const {
        return _mapTile;
    }
    
    std::shared_ptr<Feature> VectorTileClickInfo::getFeature() const {
        return _feature;
    }
    
    std::shared_ptr<Layer> VectorTileClickInfo::getLayer() const {
        return _layer;
    }

}
