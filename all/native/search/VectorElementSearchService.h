/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORELEMENTSEARCHSERVICE_H_
#define _CARTO_VECTORELEMENTSEARCHSERVICE_H_

#ifdef _CARTO_SEARCH_SUPPORT

#include "search/SearchRequest.h"

#include <memory>
#include <vector>

namespace carto {
    class Projection;
    class VectorElement;
    class VectorDataSource;

    /**
     * A search service for finding vector elements from the specified vector data source.
     */
    class VectorElementSearchService {
    public:
        /**
         * Constructs a VectorElementSearchService for the given vector data source.
         * @param dataSource The vector data source to search from.
         */
        explicit VectorElementSearchService(const std::shared_ptr<VectorDataSource>& dataSource);
        virtual ~VectorElementSearchService();

        /**
         * Returns the vector data source of the search service.
         * @return The vector data source of the search service.
         */
        const std::shared_ptr<VectorDataSource>& getDataSource() const;

        /**
         * Searches for the vector elements specified by search request from the data source bound to the service.
         * @param request The search request containing search filters.
         * @return The resulting list of vector elements matching the request.
         */
        virtual std::vector<std::shared_ptr<VectorElement> > findElements(const std::shared_ptr<SearchRequest>& request) const;

    protected:
        const std::shared_ptr<VectorDataSource> _dataSource;
    };
    
}

#endif

#endif
