#ifdef _CARTO_ROUTING_SUPPORT

#include "RouteMatchingEdge.h"

#include <sstream>

namespace carto {

    RouteMatchingEdge::RouteMatchingEdge() :
        _attributes()
    {
    }

    RouteMatchingEdge::RouteMatchingEdge(const std::map<std::string, Variant>& attributes) :
        _attributes(attributes)
    {
    }

    RouteMatchingEdge::~RouteMatchingEdge() {
    }

    bool RouteMatchingEdge::containsAttribute(const std::string& name) const {
        return _attributes.find(name) != _attributes.end();
    }

    Variant RouteMatchingEdge::getAttribute(const std::string& name) const {
        auto it = _attributes.find(name);
        if (it != _attributes.end()) {
            return it->second;
        }
        return Variant();
    }

    std::string RouteMatchingEdge::toString() const {
        std::stringstream ss;
        ss << "RouteMatchingEdge [";
        for (auto it = _attributes.begin(); it != _attributes.end(); it++) {
            ss << (it != _attributes.begin() ? ", " : "") << it->first << "=" << it->second.toString();
        }
        ss << "]";
        return ss.str();
    }

}

#endif
