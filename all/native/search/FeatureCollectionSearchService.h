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

    class FeatureCollectionSearchService {
    public:
        FeatureCollectionSearchService(const std::shared_ptr<Projection>& projection, const std::shared_ptr<FeatureCollection>& featureCollection);
        virtual ~FeatureCollectionSearchService();

        const std::shared_ptr<Projection>& getProjection() const;
        const std::shared_ptr<FeatureCollection>& getFeatureCollection() const;

        virtual std::shared_ptr<FeatureCollection> findFeatures(const std::shared_ptr<SearchRequest>& request) const;

    protected:
        const std::shared_ptr<Projection> _projection;
        const std::shared_ptr<FeatureCollection> _featureCollection;
    };
    
}

#endif

#endif
