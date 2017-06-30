#include "VectorTileClickInfo.h"

namespace carto {

    VectorTileClickInfo::VectorTileClickInfo(ClickType::ClickType clickType, const MapPos& clickPos, const MapPos& featureClickPos, const MapTile& mapTile, 
                                             const std::shared_ptr<VectorTileFeature>& vectorTileFeature,
                                             const std::shared_ptr<Layer>& layer) :
        _clickType(clickType),
        _clickPos(clickPos),
        _featureClickPos(featureClickPos),
        _mapTile(mapTile),
        _vectorTileFeature(vectorTileFeature),
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

    std::shared_ptr<VectorTileFeature> VectorTileClickInfo::getVectorTileFeature() const {
        return _vectorTileFeature;
    }
    
    long long VectorTileClickInfo::getFeatureId() const {
        return _vectorTileFeature->getId();
    }
    
    std::shared_ptr<Feature> VectorTileClickInfo::getFeature() const {
        return std::make_shared<Feature>(_vectorTileFeature->getGeometry(), _vectorTileFeature->getProperties());
    }
    
    const std::string& VectorTileClickInfo::getFeatureLayerName() const {
        return _vectorTileFeature->getLayerName();
    }
    
    std::shared_ptr<Layer> VectorTileClickInfo::getLayer() const {
        return _layer;
    }

}
