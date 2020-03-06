#ifdef _CARTO_ROUTING_SUPPORT

#include "RouteMatchingRequest.h"
#include "components/Exceptions.h"

#include <iomanip>
#include <sstream>

#include <boost/algorithm/string.hpp>

namespace carto {

    RouteMatchingRequest::RouteMatchingRequest(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points, float accuracy) :
        _projection(projection),
        _points(points),
        _accuracy(accuracy),
        _customParams(),
        _mutex()
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

    Variant RouteMatchingRequest::getCustomParameters() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _customParams;
    }

    Variant RouteMatchingRequest::getCustomParameter(const std::string& param) const {
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

    void RouteMatchingRequest::setCustomParameter(const std::string& param, const Variant& value) {
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

    std::string RouteMatchingRequest::toString() const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "RouteMatchingRequest [points=[";
        for (auto it = _points.begin(); it != _points.end(); ++it) {
            const MapPos& pos = *it;
            ss << (it == _points.begin() ? "" : ", ") << pos.toString();
        }
        ss << "], accuracy=" << _accuracy;
        if (_customParams.getType() != VariantType::VARIANT_TYPE_NULL) {
            ss << ", customParams=" << _customParams.toString();
        }
        ss << "]";
        return ss.str();
    }

}

#endif
