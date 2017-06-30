#include "VectorTileFeatureCollection.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

namespace carto {

    VectorTileFeatureCollection::VectorTileFeatureCollection(std::vector<std::shared_ptr<VectorTileFeature> > features) :
        _features(std::move(features))
    {
    }

    VectorTileFeatureCollection::~VectorTileFeatureCollection() {
    }

    int VectorTileFeatureCollection::getFeatureCount() const {
        return static_cast<int>(_features.size());
    }
    
    std::shared_ptr<VectorTileFeature> VectorTileFeatureCollection::getFeature(int index) const {
        if (index < 0 || index >= static_cast<int>(_features.size())) {
            throw OutOfRangeException("Feature index out of range");
        }
        return _features[index];
    }

}
