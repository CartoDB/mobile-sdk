#include "VectorTileFeatureCollection.h"

namespace carto {

    VectorTileFeatureCollection::VectorTileFeatureCollection(const std::vector<std::shared_ptr<VectorTileFeature> >& features) :
        FeatureCollection(features.begin(), features.end())
    {
    }

    VectorTileFeatureCollection::~VectorTileFeatureCollection() {
    }

    std::shared_ptr<VectorTileFeature> VectorTileFeatureCollection::getFeature(int index) const {
        return std::static_pointer_cast<VectorTileFeature>(FeatureCollection::getFeature(index));
    }

}
