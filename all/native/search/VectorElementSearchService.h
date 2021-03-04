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
#include <mutex>
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
         * Returns the maximum number of results the search service returns.
         * @return The maximum number of results the search service returns.
         */
        int getMaxResults() const;
        /**
         * Sets the maximum number of results the search service returns.
         * The default number of results is 1000.
         * @param maxResults The new maximum number of results the geocoding service returns.
         */
        void setMaxResults(int maxResults);

        /**
         * Searches for the vector elements specified by search request from the data source bound to the service.
         * Depending on the data source, this method may perform slow IO operations and may need to be run in background thread.
         * @param request The search request containing search filters.
         * @return The resulting list of vector elements matching the request.
         */
        virtual std::vector<std::shared_ptr<VectorElement> > findElements(const std::shared_ptr<SearchRequest>& request) const;

    protected:
        const std::shared_ptr<VectorDataSource> _dataSource;

        int _maxResults;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
