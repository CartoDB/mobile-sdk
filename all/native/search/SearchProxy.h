/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SEARCHPROXY_H_
#define _CARTO_SEARCHPROXY_H_

#ifdef _CARTO_SEARCH_SUPPORT

#include "core/MapBounds.h"
#include "search/SearchRequest.h"

#include <memory>
#include <vector>
#include <regex>

#include <boost/optional.hpp>

namespace carto {
    class Geometry;
    class Projection;
    class QueryExpression;
    class Variant;

    class SearchProxy {
    public:
        SearchProxy(const std::shared_ptr<SearchRequest>& request, const MapBounds& mapBounds, const std::shared_ptr<Projection>& proj);

        const MapBounds& getSearchBounds() const;

        bool testBounds(const MapBounds& bounds) const;

        bool testElement(const std::shared_ptr<Geometry>& geometry, const std::string* layerName, const Variant& var) const;

    protected:
        std::shared_ptr<SearchRequest> _request;
        std::shared_ptr<Geometry> _geometry;
        MapBounds _searchBounds;
        double _searchRadius;
        std::shared_ptr<Projection> _projection;
        std::shared_ptr<QueryExpression> _expr;
        boost::optional<std::regex> _re;
    };
    
}

#endif

#endif
