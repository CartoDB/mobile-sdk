#ifdef _CARTO_ROUTING_SUPPORT

#include "RoutingRequest.h"
#include "components/Exceptions.h"

#include <iomanip>
#include <sstream>

namespace carto {

    RoutingRequest::RoutingRequest(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points) :
        _projection(projection),
        _points(points)
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

    std::string RoutingRequest::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "RoutingRequest [points=";
        for (auto it = _points.begin(); it != _points.end(); ++it) {
            const MapPos& pos = *it;
            ss << (it == _points.begin() ? "" : ", ") << pos.toString();
        }
        ss << "]";
        return ss.str();
    }

}

#endif
