#ifdef _CARTO_GEOCODING_SUPPORT

#include "ReverseGeocodingRequest.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"

namespace carto {

    ReverseGeocodingRequest::ReverseGeocodingRequest(const std::shared_ptr<Projection>& projection, const MapPos& location) :
        _location(location),
        _searchRadius(DEFAULT_SEARCH_RADIUS),
        _projection(projection),
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

    std::string ReverseGeocodingRequest::toString() const {
        return "ReverseGeocodingRequest [location=" + _location.toString() + "]";
    }
    
    const float ReverseGeocodingRequest::DEFAULT_SEARCH_RADIUS = 100;

}

#endif
