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
        _pointParams(),
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

    Variant RoutingRequest::getPointParameters(int index) const {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _pointParams.find(index);
        if (it == _pointParams.end()) {
            return Variant();
        }
        return it->second;
    }

    Variant RoutingRequest::getPointParameter(int index, const std::string& param) const {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _pointParams.find(index);
        if (it == _pointParams.end()) {
            return Variant();
        }
        std::vector<std::string> keys;
        boost::split(keys, param, boost::is_any_of("."));
        picojson::value subValue = it->second.toPicoJSON();
        for (const std::string& key : keys) {
            if (!subValue.is<picojson::object>()) {
                return Variant();
            }
            subValue = subValue.get(key);
        }
        return Variant::FromPicoJSON(subValue);
    }

    void RoutingRequest::setPointParameter(int index, const std::string& param, const Variant& value) {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> keys;
        boost::split(keys, param, boost::is_any_of("."));
        picojson::value rootValue = _pointParams[index].toPicoJSON();
        picojson::value* subValue = &rootValue;
        for (const std::string& key : keys) {
            if (!subValue->is<picojson::object>()) {
                subValue->set(picojson::object());
            }
            subValue = &subValue->get<picojson::object>()[key];
        }
        *subValue = value.toPicoJSON();
        _pointParams[index] = Variant::FromPicoJSON(rootValue);
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
        picojson::value rootValue = _customParams.toPicoJSON();
        picojson::value* subValue = &rootValue;
        for (const std::string& key : keys) {
            if (!subValue->is<picojson::object>()) {
                subValue->set(picojson::object());
            }
            subValue = &subValue->get<picojson::object>()[key];
        }
        *subValue = value.toPicoJSON();
        _customParams = Variant::FromPicoJSON(rootValue);
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
        if (!_pointParams.empty()) {
            ss << ", pointParams=[";
            for (auto it = _pointParams.begin(); it != _pointParams.end(); ++it) {
                ss << (it == _pointParams.begin() ? "" : ", ") << it->first << "=" << it->second.toString();
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
