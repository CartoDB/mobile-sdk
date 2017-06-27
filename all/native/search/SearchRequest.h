/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SEARCHREQUEST_H_
#define _CARTO_SEARCHREQUEST_H_

#ifdef _CARTO_SEARCH_SUPPORT

#include <memory>
#include <mutex>
#include <string>

namespace carto {
    class Geometry;
    class Projection;

    class SearchRequest {
    public:
        explicit SearchRequest(const std::shared_ptr<Projection>& projection);
        virtual ~SearchRequest();

        const std::shared_ptr<Projection>& getProjection() const;

        std::string getFilterExpression() const;
        void setFilterExpression(const std::string& expr);

        std::string getRegexFilter() const;
        void setRegexFilter(const std::string& regex);

        std::shared_ptr<Geometry> getGeometry() const;
        void setGeometry(const std::shared_ptr<Geometry>& geometry);

        float getSearchRadius() const;
        void setSearchRadius(float radius);

        std::string toString() const;

    private:
        static const int DEFAULT_SEARCH_RADIUS = 0; // in meters

        const std::shared_ptr<Projection> _projection;
        std::string _filterExpr;
        std::string _regexFilter;
        std::shared_ptr<Geometry> _geometry;
        float _searchRadius;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
