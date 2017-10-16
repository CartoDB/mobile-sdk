#ifdef _CARTO_ROUTING_SUPPORT

#include "RouteMatchingRequest.h"
#include "components/Exceptions.h"

#include <iomanip>
#include <sstream>

namespace carto {

    RouteMatchingRequest::RouteMatchingRequest(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points, float accuracy) :
        _projection(projection),
        _points(points),
        _accuracy(accuracy)
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
    }

    RouteMatchingRequest::~RouteMatchingRequest() {
    }

    const std::shared_ptr<Projection>& RouteMatchingRequest::getProjection() const {
        return _projection;
    }

    const std::vector<MapPos>& RouteMatchingRequest::getPoints() const {
        return _points;
    }

    float RouteMatchingRequest::getAccuracy() const {
        return _accuracy;
    }

    std::string RouteMatchingRequest::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "RouteMatchingRequest [points=";
        for (auto it = _points.begin(); it != _points.end(); ++it) {
            const MapPos& pos = *it;
            ss << (it == _points.begin() ? "" : ", ") << pos.toString();
        }
        ss << ", accuracy=" << _accuracy;
        ss << "]";
        return ss.str();
    }

}

#endif
