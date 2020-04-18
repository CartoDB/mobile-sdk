#ifdef _CARTO_GEOCODING_SUPPORT

#include "GeocodingRequest.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"

#include <cmath>
#include <limits>
#include <iomanip>
#include <sstream>

#include <boost/algorithm/string.hpp>

namespace carto {

    GeocodingRequest::GeocodingRequest(const std::shared_ptr<Projection>& projection, const std::string& query) :
        _query(query),
        _projection(projection),
        _location(0, 0),
        _locationDefined(false),
        _locationRadius(0),
        _customParams(),
        _mutex()
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
    }

    GeocodingRequest::~GeocodingRequest() {
    }

    const std::string& GeocodingRequest::getQuery() const {
        return _query;
    }

    const std::shared_ptr<Projection>& GeocodingRequest::getProjection() const {
        return _projection;
    }

    MapPos GeocodingRequest::getLocation() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _location;
    }

    void GeocodingRequest::setLocation(const MapPos& pos) {
        std::lock_guard<std::mutex> lock(_mutex);
        _location = pos;
        _locationDefined = true;
    }

    bool GeocodingRequest::isLocationDefined() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _locationDefined;
    }

    float GeocodingRequest::getLocationRadius() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _locationRadius;
    }

    void GeocodingRequest::setLocationRadius(float radius) {
        std::lock_guard<std::mutex> lock(_mutex);
        _locationRadius = radius;
    }

    Variant GeocodingRequest::getCustomParameters() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _customParams;
    }

    Variant GeocodingRequest::getCustomParameter(const std::string& param) const {
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

    void GeocodingRequest::setCustomParameter(const std::string& param, const Variant& value) {
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

    std::string GeocodingRequest::toString() const {
        std::lock_guard<std::mutex> lock(_mutex);

        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "GeocodingRequest [";
        ss << "query='" << _query << "'";
        if (_locationDefined) {
            ss << ", location=" << _location.toString();
        }
        if (_locationRadius > 0) {
            ss << ", locationRadius=" << _locationRadius;
        }
        if (_customParams.getType() != VariantType::VARIANT_TYPE_NULL) {
            ss << ", customParams=" << _customParams.toString();
        }
        ss << "]";
        return ss.str();
    }
    
}

#endif
