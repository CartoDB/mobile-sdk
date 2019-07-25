#ifdef _CARTO_ROUTING_SUPPORT

#include "RoutingRequest.h"
#include "components/Exceptions.h"

#include <iomanip>
#include <sstream>

namespace carto {

    RoutingRequest::RoutingRequest(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points) :
        _projection(projection),
        _points(points),
        _filters()
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
    }

    RoutingRequest::RoutingRequest(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points, const std::vector<Variant>& filters) :
        _projection(projection),
        _points(points),
        _filters(filters)
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }

        if (!filters.empty() && filters.size() != _points.size()) {
            throw InvalidArgumentException("Points and filters size mismatch");
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

    const std::vector<Variant>& RoutingRequest::getFilters() const {
        return _filters;
    }

    std::string RoutingRequest::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "RoutingRequest [points=";
        for (auto it = _points.begin(); it != _points.end(); ++it) {
            const MapPos& pos = *it;
            ss << (it == _points.begin() ? "" : ", ") << pos.toString();
        }
        if (!_filters.empty()) {
            ss << ", filters=";
            for (auto it = _filters.begin(); it != _filters.end(); ++it) {
                const Variant& filter = *it;
                ss << (it == _filters.begin() ? "" : ", ") << filter.toString();
            }
        }
        ss << "]";
        return ss.str();
    }

}

#endif
