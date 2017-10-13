#ifdef _CARTO_ROUTING_SUPPORT

#include "RouteMatchingResult.h"
#include "components/Exceptions.h"

#include <numeric>
#include <functional>
#include <utility>
#include <iomanip>
#include <sstream>

namespace carto {

    RouteMatchingResult::RouteMatchingResult(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points) :
        _projection(projection),
        _points(points)
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
    }

    RouteMatchingResult::~RouteMatchingResult() {
    }

    const std::shared_ptr<Projection>& RouteMatchingResult::getProjection() const {
        return _projection;
    }

    const std::vector<MapPos>& RouteMatchingResult::getPoints() const {
        return _points;
    }

    std::string RouteMatchingResult::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "RouteMatchingResult [";
        ss << "points=";
        for (auto it = _points.begin(); it != _points.end(); ++it) {
            const MapPos& pos = *it;
            ss << (it == _points.begin() ? "" : ", ") << pos.toString();
        }
        ss << "]";
        return ss.str();
    }

}

#endif
