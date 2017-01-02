#ifdef _CARTO_GEOCODING_SUPPORT

#include "ReverseGeocodingRequest.h"
#include "core/Exceptions.h"
#include "projection/Projection.h"

namespace carto {

    ReverseGeocodingRequest::ReverseGeocodingRequest(const std::shared_ptr<Projection>& projection, const MapPos& point) :
        _point(point),
        _projection(projection)
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
    }

    ReverseGeocodingRequest::~ReverseGeocodingRequest() {
    }

    const MapPos& ReverseGeocodingRequest::getPoint() const {
        return _point;
    }

    const std::shared_ptr<Projection>& ReverseGeocodingRequest::getProjection() const {
        return _projection;
    }

    std::string ReverseGeocodingRequest::toString() const {
        return "ReverseGeocodingRequest [point=" + _point.toString() + "]";
    }
    
}

#endif
