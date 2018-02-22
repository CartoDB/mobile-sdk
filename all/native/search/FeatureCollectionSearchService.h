/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_FEATURECOLLECTIONSEARCHSERVICE_H_
#define _CARTO_FEATURECOLLECTIONSEARCHSERVICE_H_

#ifdef _CARTO_SEARCH_SUPPORT

#include "search/SearchRequest.h"

#include <memory>
#include <vector>

namespace carto {
    class FeatureCollection;
    class Projection;

    /**
     * A search service for finding features from a specified feature collection.
     */
    class FeatureCollectionSearchService {
    public:
        /**
         * Constructs a FeatureCollectionSearchService for given feature collection.
         * @param projection The projection to use for the feature collection.
         * @param featureCollection The feature collection to search from.
         */
        FeatureCollectionSearchService(const std::shared_ptr<Projection>& projection, const std::shared_ptr<FeatureCollection>& featureCollection);
        virtual ~FeatureCollectionSearchService();

        /**
         * Returns the projection of the feature collection of the search service.
         * @return The projection of the feature collection of the search service.
         */
        const std::shared_ptr<Projection>& getProjection() const;

        /**
         * Returns the feature collection of the search service.
         * @return The feature collection of the search service.
         */
        const std::shared_ptr<FeatureCollection>& getFeatureCollection() const;

        /**
         * Searches for the features specified by search request from the feature collection bound to the service.
         * @param request The search request containing search filters.
         * @return The resulting feature collection containing features matching the request.
         */
        virtual std::shared_ptr<FeatureCollection> findFeatures(const std::shared_ptr<SearchRequest>& request) const;

    protected:
        const std::shared_ptr<Projection> _projection;
        const std::shared_ptr<FeatureCollection> _featureCollection;
    };
    
}

#endif

#endif
