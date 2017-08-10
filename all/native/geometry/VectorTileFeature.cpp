#include "VectorTileFeature.h"

namespace carto {

    VectorTileFeature::VectorTileFeature(long long id, const MapTile& mapTile, const std::string& layerName, const std::shared_ptr<Geometry>& geometry, Variant properties) :
        Feature(geometry, std::move(properties)),
        _id(id),
        _mapTile(mapTile),
        _layerName(layerName)
    {
    }
    
    VectorTileFeature::~VectorTileFeature() {
    }

    long long VectorTileFeature::getId() const {
        return _id;
    }

    const MapTile& VectorTileFeature::getMapTile() const {
        return _mapTile;
    }

    const std::string& VectorTileFeature::getLayerName() const {
        return _layerName;
    }
    
}
