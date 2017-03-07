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
        _location(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()),
        _locationRadius(DEFAULT_LOCATION_RADIUS),
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
        if (!std::isnan(_location.getX()) && !std::isnan(_location.getY())) {
            ss << ", location=" << _location.toString();
            ss << ", locationRadius=" << _locationRadius;
        }
        ss << "]";
        return ss.str();
    }
    
}

#endif
