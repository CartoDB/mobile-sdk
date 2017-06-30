#include "VectorTileFeature.h"
#include "geometry/Geometry.h"

namespace carto {

    VectorTileFeature::VectorTileFeature(long long id, const std::string& layerName, const std::shared_ptr<Geometry>& geometry, Variant properties) :
        _id(id),
        _layerName(layerName),
        _geometry(geometry),
        _properties(std::move(properties))
    {
    }
    
    VectorTileFeature::~VectorTileFeature() {
    }

    long long VectorTileFeature::getId() const {
        return _id;
    }

    const std::string& VectorTileFeature::getLayerName() const {
        return _layerName;
    }
    
    const std::shared_ptr<Geometry>& VectorTileFeature::getGeometry() const {
        return _geometry;
    }
    
    const Variant& VectorTileFeature::getProperties() const {
        return _properties;
    }
    
}
