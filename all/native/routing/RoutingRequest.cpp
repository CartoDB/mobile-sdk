#ifdef _CARTO_ROUTING_SUPPORT

#include "RoutingRequest.h"
#include "components/Exceptions.h"

#include <iomanip>
#include <sstream>

#include <boost/algorithm/string.hpp>

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

    Variant RoutingRequest::getCustomParameters() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _customParams;
    }

    Variant RoutingRequest::getCustomParameter(const std::string& param) const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> keys;
        boost::split(keys, param, boost::is_any_of("."));
        picojson::value subValue = _customParams.toPicoJSON();
        for (const std::string& key : keys) {
            if (!subValue.is<picojson::object>()) {
                return Variant();
            }
            subValue = subValue.get(key);
        }
        return Variant::FromPicoJSON(subValue);
    }

    void RoutingRequest::setCustomParameter(const std::string& param, const Variant& value) {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> keys;
        boost::split(keys, param, boost::is_any_of("."));
        picojson::value config = _customParams.toPicoJSON();
        picojson::value* subValue = &config;
        for (const std::string& key : keys) {
            if (!subValue->is<picojson::object>()) {
                subValue->set(picojson::object());
            }
            subValue = &subValue->get<picojson::object>()[key];
        }
        *subValue = value.toPicoJSON();
        _customParams = Variant::FromPicoJSON(config);
    }

    std::string RoutingRequest::toString() const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "RoutingRequest [points=[";
        for (auto it = _points.begin(); it != _points.end(); ++it) {
            ss << (it == _points.begin() ? "" : ", ") << it->toString();
        }
        ss << "]";
        if (!_filters.empty()) {
            ss << ", filters=[";
            for (auto it = _filters.begin(); it != _filters.end(); ++it) {
                ss << (it == _filters.begin() ? "" : ", ") << it->toString();
            }
            ss << "]";
        }
        if (_customParams.getType() != VariantType::VARIANT_TYPE_NULL) {
            ss << ", customParams=" << _customParams.toString();
        }
        ss << "]";
        return ss.str();
    }

}

#endif
