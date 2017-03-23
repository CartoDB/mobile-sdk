#ifdef _CARTO_GEOCODING_SUPPORT

#include "ReverseGeocodingRequest.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"

namespace carto {

    ReverseGeocodingRequest::ReverseGeocodingRequest(const std::shared_ptr<Projection>& projection, const MapPos& location) :
        _location(location),
        _projection(projection)
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

    const std::shared_ptr<Projection>& ReverseGeocodingRequest::getProjection() const {
        return _projection;
    }

    std::string ReverseGeocodingRequest::toString() const {
        return "ReverseGeocodingRequest [location=" + _location.toString() + "]";
    }
    
}

#endif
