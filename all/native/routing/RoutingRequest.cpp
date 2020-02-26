#ifdef _CARTO_ROUTING_SUPPORT

#include "RoutingRequest.h"
#include "components/Exceptions.h"

#include <iomanip>
#include <sstream>

namespace carto {

    RoutingRequest::RoutingRequest(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points) :
        _projection(projection),
        _points(points),
        _filters(),
        _customParams(),
        _mutex()
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
    }

    RoutingRequest::~RoutingRequest() {
    }

    const std::shared_ptr<Projection>& RoutingRequest::getProjection() const {
        return _projection;
    }

    const std::vector<MapPos>& RoutingRequest::getPoints() const {
        return _points;
    }

    std::vector<Variant> RoutingRequest::getGeometryTagFilters() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _filters;
    }

    void RoutingRequest::setGeometryTagFilters(const std::vector<Variant>& filters) {
        std::lock_guard<std::mutex> lock(_mutex);
        _filters = filters;
    }

    std::map<std::string, Variant> RoutingRequest::getCustomParameters() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _customParams;
    }

    void RoutingRequest::setCustomParameters(const std::map<std::string, Variant>& customParams) {
        std::lock_guard<std::mutex> lock(_mutex);
        _customParams = customParams;
    }

    std::string RoutingRequest::toString() const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "RoutingRequest [points=";
        for (auto it = _points.begin(); it != _points.end(); ++it) {
            ss << (it == _points.begin() ? "" : ", ") << it->toString();
        }
        if (!_filters.empty()) {
            ss << ", filters=";
            for (auto it = _filters.begin(); it != _filters.end(); ++it) {
                ss << (it == _filters.begin() ? "" : ", ") << it->toString();
            }
        }
        if (!_customParams.empty()) {
            ss << ", customParams=";
            for (auto it = _customParams.begin(); it != _customParams.end(); ++it) {
                ss << (it == _customParams.begin() ? "" : ", ") << it->first << "=" << it->second.toString();
            }
        }
        ss << "]";
        return ss.str();
    }

}

#endif
