#ifdef _CARTO_ROUTING_SUPPORT

#include "RouteMatchingPoint.h"

#include <sstream>

namespace carto {

    RouteMatchingPoint::RouteMatchingPoint() :
        _pos(),
        _type(RouteMatchingPointType::ROUTE_MATCHING_POINT_UNMATCHED),
        _edgeIndex(-1)
    {
    }

    RouteMatchingPoint::RouteMatchingPoint(const MapPos& pos, RouteMatchingPointType::RouteMatchingPointType type, int edgeIndex) :
        _pos(pos),
        _type(type),
        _edgeIndex(edgeIndex)
    {
    }

    RouteMatchingPoint::~RouteMatchingPoint() {
    }

    const MapPos& RouteMatchingPoint::getPos() const {
        return _pos;
    }

    RouteMatchingPointType::RouteMatchingPointType RouteMatchingPoint::getType() const {
        return _type;
    }

    int RouteMatchingPoint::getEdgeIndex() const {
        return _edgeIndex;
    }

    std::string RouteMatchingPoint::toString() const {
        std::string typeName;
        switch (_type) {
        case RouteMatchingPointType::ROUTE_MATCHING_POINT_UNMATCHED:
            typeName = "unmatched";
            break;
        case RouteMatchingPointType::ROUTE_MATCHING_POINT_INTERPOLATED:
            typeName = "interpolated";
            break;
        case RouteMatchingPointType::ROUTE_MATCHING_POINT_MATCHED:
            typeName = "matched";
            break;
        }

        std::stringstream ss;
        ss << "RouteMatchingPoint [type=" << typeName << ", pos=" << _pos.toString() << "]";
        return ss.str();
    }

}

#endif
