#include "VectorTileClickInfo.h"

namespace carto {

    VectorTileClickInfo::VectorTileClickInfo(ClickType::ClickType clickType, const MapPos& clickPos, const MapPos& featureClickPos, const MapTile& mapTile,
                                                   long long featureId,
                                                   const std::shared_ptr<Feature>& feature,
                                                   const std::string& featureLayerName,
                                                   const std::shared_ptr<Layer>& layer) :
        _clickType(clickType),
        _clickPos(clickPos),
        _featureClickPos(featureClickPos),
        _mapTile(mapTile),
        _featureId(featureId),
        _feature(feature),
        _featureLayerName(featureLayerName),
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

    long long VectorTileClickInfo::getFeatureId() const {
        return _featureId;
    }
    
    std::shared_ptr<Feature> VectorTileClickInfo::getFeature() const {
        return _feature;
    }
    
    const std::string& VectorTileClickInfo::getFeatureLayerName() const {
        return _featureLayerName;
    }
    
    std::shared_ptr<Layer> VectorTileClickInfo::getLayer() const {
        return _layer;
    }

}
