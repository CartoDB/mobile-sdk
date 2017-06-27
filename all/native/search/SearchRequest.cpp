#ifdef _CARTO_SEARCH_SUPPORT

#include "SearchRequest.h"
#include "components/Exceptions.h"
#include "geometry/Geometry.h"
#include "geometry/GeoJSONGeometryWriter.h"
#include "projections/Projection.h"

#include <sstream>

namespace carto {

    SearchRequest::SearchRequest(const std::shared_ptr<Projection>& projection) :
        _projection(projection),
        _filterExpr(),
        _regexFilter(),
        _geometry(),
        _searchRadius(DEFAULT_SEARCH_RADIUS),
        _mutex()
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
    }

    SearchRequest::~SearchRequest() {
    }

    const std::shared_ptr<Projection>& SearchRequest::getProjection() const {
         return _projection;
    }

    std::string SearchRequest::getFilterExpression() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _filterExpr;
    }

    void SearchRequest::setFilterExpression(const std::string& expr) {
        std::lock_guard<std::mutex> lock(_mutex);
        _filterExpr = expr;
    }

    std::string SearchRequest::getRegexFilter() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _regexFilter;
    }

    void SearchRequest::setRegexFilter(const std::string& regex) {
        std::lock_guard<std::mutex> lock(_mutex);
        _regexFilter = regex;
    }

    std::shared_ptr<Geometry> SearchRequest::getGeometry() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _geometry;
    }

    void SearchRequest::setGeometry(const std::shared_ptr<Geometry>& geometry) {
        std::lock_guard<std::mutex> lock(_mutex);
        _geometry = geometry;
    }

    float SearchRequest::getSearchRadius() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _searchRadius;
    }

    void SearchRequest::setSearchRadius(float radius) {
        std::lock_guard<std::mutex> lock(_mutex);
        _searchRadius = radius;
    }

    std::string SearchRequest::toString() const {
        std::lock_guard<std::mutex> lock(_mutex);

        std::stringstream ss;
        ss << "SearchRequest [";
        ss << "filter='" << _filterExpr << "'";
        ss << ", regex='" << _regexFilter << "'";
        if (_geometry) {
            ss << ", geometry=" << GeoJSONGeometryWriter().writeGeometry(_geometry);
            ss << ", searchRadius=" << _searchRadius;
        }
        ss << "]";
        return ss.str();
    }
    
}

#endif
