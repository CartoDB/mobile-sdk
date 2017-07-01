#ifdef _CARTO_SEARCH_SUPPORT

#include "FeatureCollectionSearchService.h"
#include "components/Exceptions.h"
#include "geometry/Geometry.h"
#include "geometry/Feature.h"
#include "geometry/FeatureCollection.h"
#include "search/SearchProxy.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "utils/Log.h"

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

        SearchProxy proxy(request, EPSG3857().getBounds());

        std::vector<std::shared_ptr<Feature> > features;
        for (int i = 0; i < _featureCollection->getFeatureCount(); i++) {
            const std::shared_ptr<Feature>& feature = _featureCollection->getFeature(i);

            if (proxy.testElement(feature->getGeometry(), _projection, feature->getProperties())) {
                features.push_back(feature);
            }
        }
        return std::make_shared<FeatureCollection>(features);
    }

}

#endif
