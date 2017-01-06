#ifdef _CARTO_GEOCODING_SUPPORT

#include "GeocodingRequest.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"

namespace carto {

    GeocodingRequest::GeocodingRequest(const std::shared_ptr<Projection>& projection, const std::string& query) :
        _query(query),
        _projection(projection)
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

    std::string GeocodingRequest::toString() const {
        return "GeocodingRequest [query='" + _query + "']";
    }
    
}

#endif
