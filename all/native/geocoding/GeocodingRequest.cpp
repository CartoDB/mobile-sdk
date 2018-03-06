#ifdef _CARTO_GEOCODING_SUPPORT

#include "GeocodingRequest.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"

#include <cmath>
#include <limits>
#include <sstream>

namespace carto {

    GeocodingRequest::GeocodingRequest(const std::shared_ptr<Projection>& projection, const std::string& query) :
        _query(query),
        _projection(projection),
        _location(0, 0),
        _locationDefined(false),
        _locationRadius(0),
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

    std::string GeocodingRequest::toString() const {
        std::lock_guard<std::mutex> lock(_mutex);

        std::stringstream ss;
        ss << "GeocodingRequest [";
        ss << "query='" << _query << "'";
        if (_locationDefined) {
            ss << ", location=" << _location.toString();
        }
        if (_locationRadius > 0) {
            ss << ", locationRadius=" << _locationRadius;
        }
        ss << "]";
        return ss.str();
    }
    
}

#endif
