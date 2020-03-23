#include "FeatureBuilder.h"
#include "geometry/Feature.h"
#include "geometry/Geometry.h"

namespace carto {

    FeatureBuilder::FeatureBuilder() :
        _geometry(),
        _propertyMap(),
        _mutex()
    {
    }
    
    FeatureBuilder::~FeatureBuilder() {
    }
    
    std::shared_ptr<Geometry> FeatureBuilder::getGeometry() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _geometry;
    }
    
    void FeatureBuilder::setGeometry(const std::shared_ptr<Geometry>& geometry) {
        std::lock_guard<std::mutex> lock(_mutex);
        _geometry = geometry;
    }
    
    Variant FeatureBuilder::getPropertyValue(const std::string& key) const {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _propertyMap.find(key);
        if (it != _propertyMap.end()) {
            return it->second;
        }
        return Variant();
    }

    void FeatureBuilder::setPropertyValue(const std::string& key, const Variant& value) {
        std::lock_guard<std::mutex> lock(_mutex);
        _propertyMap[key] = value;
    }

    std::shared_ptr<Feature> FeatureBuilder::buildFeature() const {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_geometry) {
            return std::shared_ptr<Feature>();
        }
        return std::make_shared<Feature>(_geometry, Variant(_propertyMap));
    }

}
