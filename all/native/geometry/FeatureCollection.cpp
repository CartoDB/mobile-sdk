#include "FeatureCollection.h"
#include "utils/Log.h"

namespace carto {

    FeatureCollection::FeatureCollection(const std::vector<std::shared_ptr<Feature> >& features) :
        _features(features)
    {
    }

    FeatureCollection::~FeatureCollection() {
    }

    int FeatureCollection::getFeatureCount() const {
        return static_cast<int>(_features.size());
    }
    
    std::shared_ptr<Feature> FeatureCollection::getFeature(int index) const {
        if (index < 0 || index >= static_cast<int>(_features.size())) {
            throw std::out_of_range("Feature index out of range");
        }
        return _features[index];
    }

}
