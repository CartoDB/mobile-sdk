#include "VectorTileFeatureBuilder.h"
#include "geometry/VectorTileFeature.h"

namespace carto {

    VectorTileFeatureBuilder::VectorTileFeatureBuilder() :
        FeatureBuilder(),
        _id(0),
        _mapTile(),
        _layerName()
    {
    }
    
    VectorTileFeatureBuilder::~VectorTileFeatureBuilder() {
    }

    long long VectorTileFeatureBuilder::getId() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _id;
    }

    void VectorTileFeatureBuilder::setId(long long id) {
        std::lock_guard<std::mutex> lock(_mutex);
        _id = id;
    }

    MapTile VectorTileFeatureBuilder::getMapTile() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _mapTile;
    }

    void VectorTileFeatureBuilder::setMapTile(const MapTile& mapTile) {
        std::lock_guard<std::mutex> lock(_mutex);
        _mapTile = mapTile;
    }

    std::string VectorTileFeatureBuilder::getLayerName() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _layerName;
    }

    void VectorTileFeatureBuilder::setLayerName(const std::string& layerName) {
        std::lock_guard<std::mutex> lock(_mutex);
        _layerName = layerName;
    }
    
    std::shared_ptr<VectorTileFeature> VectorTileFeatureBuilder::buildVectorTileFeature() const {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_geometry) {
            return std::shared_ptr<VectorTileFeature>();
        }
        return std::make_shared<VectorTileFeature>(_id, _mapTile, _layerName, _geometry, Variant(_propertyMap));
    }

}
