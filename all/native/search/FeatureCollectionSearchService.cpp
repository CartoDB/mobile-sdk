#ifdef _CARTO_SEARCH_SUPPORT

#include "FeatureCollectionSearchService.h"
#include "components/Exceptions.h"
#include "geometry/Feature.h"
#include "geometry/FeatureCollection.h"
#include "projections/Projection.h"

namespace carto {

    FeatureCollectionSearchService::FeatureCollectionSearchService(const std::shared_ptr<Projection>& projection, const std::shared_ptr<FeatureCollection>& featureCollection) :
        _projection(projection),
        _featureCollection(featureCollection)
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
        if (!featureCollection) {
            throw NullArgumentException("Null featureCollection");
        }
    }

    FeatureCollectionSearchService::~FeatureCollectionSearchService() {
    }

    const std::shared_ptr<Projection>& FeatureCollectionSearchService::getProjection() const {
        return _projection;
    }

    const std::shared_ptr<FeatureCollection>& FeatureCollectionSearchService::getFeatureCollection() const {
        return _featureCollection;
    }

    std::shared_ptr<FeatureCollection> FeatureCollectionSearchService::findFeatures(const std::shared_ptr<SearchRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        return _featureCollection; // TODO: implement
    }

}

#endif
