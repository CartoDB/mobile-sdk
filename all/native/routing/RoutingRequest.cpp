#ifdef _CARTO_ROUTING_SUPPORT

#include "RoutingRequest.h"
#include "components/Exceptions.h"

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

}

#endif
