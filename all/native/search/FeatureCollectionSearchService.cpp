#ifdef _CARTO_SEARCH_SUPPORT

#include "FeatureCollectionSearchService.h"
#include "components/Exceptions.h"
#include "geometry/Geometry.h"
#include "geometry/Feature.h"
#include "geometry/FeatureCollection.h"
#include "search/utils/SearchProxy.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "utils/Log.h"

namespace carto {

    FeatureCollectionSearchService::FeatureCollectionSearchService(const std::shared_ptr<Projection>& projection, const std::shared_ptr<FeatureCollection>& featureCollection) :
        _projection(projection),
        _featureCollection(featureCollection),
        _maxResults(1000),
        _mutex()
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

    int FeatureCollectionSearchService::getMaxResults() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _maxResults;
    }

    void FeatureCollectionSearchService::setMaxResults(int maxResults) {
        std::lock_guard<std::mutex> lock(_mutex);
        _maxResults = maxResults;
    }

    std::shared_ptr<FeatureCollection> FeatureCollectionSearchService::findFeatures(const std::shared_ptr<SearchRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        SearchProxy proxy(request, _projection->getBounds(), _projection);
        int maxResults = 0;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            maxResults = _maxResults;
        }

        std::vector<std::shared_ptr<Feature> > features;
        for (int i = 0; i < _featureCollection->getFeatureCount(); i++) {
            if (static_cast<int>(features.size()) >= maxResults) {
                break;
            }

            const std::shared_ptr<Feature>& feature = _featureCollection->getFeature(i);

            if (proxy.testElement(feature->getGeometry(), nullptr, feature->getProperties())) {
                features.push_back(feature);
            }
        }
        return std::make_shared<FeatureCollection>(features);
    }

}

#endif
