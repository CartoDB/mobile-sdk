/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORDATASOURCESEARCHSERVICE_H_
#define _CARTO_VECTORDATASOURCESEARCHSERVICE_H_

#ifdef _CARTO_SEARCH_SUPPORT

#include "search/SearchRequest.h"

#include <memory>
#include <vector>

namespace carto {
    class Projection;
    class VectorElement;
    class VectorDataSource;

    class VectorDataSourceSearchService {
    public:
        VectorDataSourceSearchService(const std::shared_ptr<VectorDataSource>& dataSource);
        virtual ~VectorDataSourceSearchService();

        const std::shared_ptr<VectorDataSource>& getDataSource() const;

        virtual std::vector<std::shared_ptr<VectorElement> > findElements(const std::shared_ptr<SearchRequest>& request) const;

    protected:
        const std::shared_ptr<VectorDataSource> _dataSource;
    };
    
}

#endif

#endif
