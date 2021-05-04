#ifdef _CARTO_ROUTING_SUPPORT

#include "RouteMatchingResult.h"
#include "components/Exceptions.h"

#include <sstream>

namespace carto {

    RouteMatchingResult::RouteMatchingResult(const std::shared_ptr<Projection>& projection, std::vector<RouteMatchingPoint> matchingPoints, std::vector<RouteMatchingEdge> matchingEdges) :
        _projection(projection),
        _matchingPoints(std::move(matchingPoints)),
        _matchingEdges(std::move(matchingEdges))
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

    std::vector<MapPos> RouteMatchingResult::getPoints() const {
        std::vector<MapPos> poses;
        poses.reserve(_matchingPoints.size());
        for (const RouteMatchingPoint& point : _matchingPoints) {
            poses.push_back(point.getPos());
        }
        return poses;
    }

    const std::vector<RouteMatchingEdge>& RouteMatchingResult::getMatchingEdges() const {
        return _matchingEdges;
    }

    const std::vector<RouteMatchingPoint>& RouteMatchingResult::getMatchingPoints() const {
        return _matchingPoints;
    }

    std::string RouteMatchingResult::toString() const {
        std::stringstream ss;
        ss << "RouteMatchingResult [matchingPoints=[";
        for (auto it = _matchingPoints.begin(); it != _matchingPoints.end(); ++it) {
            ss << (it == _matchingPoints.begin() ? "" : ", ") << (*it).toString();
        }
        ss << "], matchingEdges=[";
        for (auto it = _matchingEdges.begin(); it != _matchingEdges.end(); ++it) {
            ss << (it == _matchingEdges.begin() ? "" : ", ") << (*it).toString();
        }
        ss << "]]";
        return ss.str();
    }

}

#endif
