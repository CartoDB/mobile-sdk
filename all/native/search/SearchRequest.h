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

    /**
     * A search request description. Contains various search filters,
     * including geometry proximity test, regular expression test for all attributes
     * and a custom SQL-like search filter.
     */
    class SearchRequest {
    public:
        /**
         * Constructs an empty SearchRequest.
         */
        SearchRequest();
        virtual ~SearchRequest();

        /**
         * Returns the string based search expression.
         * If empty, then search expression is not used.
         * @return The string based search expression.
         */
        std::string getFilterExpression() const;
        /**
         * Sets the string based search expression.
         * A various conditions based on the fields and geometry can be used.
         * For example, "name='X' OR (name='Y' AND gender IS NOT NULL)" is a valid expression,
         * assuming elements contains fields 'name' and 'gender'.
         * Note: This feature is currently in experimental state and may change in the future!
         * @param expr The string based expression to use.
         */
        void setFilterExpression(const std::string& expr);

        /**
         * Returns the regular expression used to search all the fields.
         * If empty, then the regular expression is not used.
         * @return The regular expression used to search all the fields.
         */
        std::string getRegexFilter() const;
        /**
         * Sets the regular expression used to search all the fields.
         * @param regex The regular expression to use.
         */
        void setRegexFilter(const std::string& regex);

        /**
         * Returns the geometry used for proximity search.
         * @return The geometry used for proximity search.
         */
        std::shared_ptr<Geometry> getGeometry() const;
        /**
         * Sets the geometry used for proximity search.
         * If geometry is set, then projection must be also defined.
         * @param geometry The geometry used for proximity search.
         */
        void setGeometry(const std::shared_ptr<Geometry>& geometry);

        /**
         * Returns the projection to use for search geometry.
         * @return The projection to use for search geometry.
         */
        std::shared_ptr<Projection> getProjection() const;
        /**
         * Sets the projection to use for search geometry.
         * @param projection The projection to use for search geometry.
         */
        void setProjection(const std::shared_ptr<Projection>& projection);

        /**
         * Returns the search radius for proximity search (in meters). The default is 0.
         * @return The proximity search radius in meters.
         */
        float getSearchRadius() const;
        /**
         * Sets the search radius for proximity search (in meters). The default is 0.
         * @param radius The proximity search radius in meters.
         */
        void setSearchRadius(float radius);

        /**
         * Creates a string representation of this request object, useful for logging.
         * @return The string representation of this request object.
         */
        std::string toString() const;

    private:
        static const int DEFAULT_SEARCH_RADIUS = 0; // in meters

        std::string _filterExpr;
        std::string _regexFilter;
        std::shared_ptr<Geometry> _geometry;
        std::shared_ptr<Projection> _projection;
        float _searchRadius;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
