#ifdef _CARTO_GEOCODING_SUPPORT

#include "ReverseGeocodingRequest.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"

#include <iomanip>
#include <sstream>

#include <boost/algorithm/string.hpp>

namespace carto {

    ReverseGeocodingRequest::ReverseGeocodingRequest(const std::shared_ptr<Projection>& projection, const MapPos& location) :
        _location(location),
        _searchRadius(DEFAULT_SEARCH_RADIUS),
        _projection(projection),
        _customParams(),
        _mutex()
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
    }

    ReverseGeocodingRequest::~ReverseGeocodingRequest() {
    }

    const MapPos& ReverseGeocodingRequest::getLocation() const {
        return _location;
    }

    float ReverseGeocodingRequest::getSearchRadius() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _searchRadius;
    }

    void ReverseGeocodingRequest::setSearchRadius(float radius) {
        std::lock_guard<std::mutex> lock(_mutex);
        _searchRadius = radius;
    }

    const std::shared_ptr<Projection>& ReverseGeocodingRequest::getProjection() const {
        return _projection;
    }

    Variant ReverseGeocodingRequest::getCustomParameters() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _customParams;
    }

    Variant ReverseGeocodingRequest::getCustomParameter(const std::string& param) const {
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

    void ReverseGeocodingRequest::setCustomParameter(const std::string& param, const Variant& value) {
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

    std::string ReverseGeocodingRequest::toString() const {
        std::lock_guard<std::mutex> lock(_mutex);

        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "ReverseGeocodingRequest [";
        ss << "location=" << _location.toString();
        if (_customParams.getType() != VariantType::VARIANT_TYPE_NULL) {
            ss << ", customParams=" << _customParams.toString();
        }
        ss << "]";
        return ss.str();
    }
    
    const float ReverseGeocodingRequest::DEFAULT_SEARCH_RADIUS = 100;

}

#endif
