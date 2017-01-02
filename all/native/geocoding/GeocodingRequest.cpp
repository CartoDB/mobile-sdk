#ifdef _CARTO_GEOCODING_SUPPORT

#include "GeocodingRequest.h"

namespace carto {

    GeocodingRequest::GeocodingRequest(const std::string& query) :
        _query(query)
    {
    }

    GeocodingRequest::~GeocodingRequest() {
    }

    const std::string& GeocodingRequest::getQuery() const {
        return _query;
    }

    std::string GeocodingRequest::toString() const {
        return "GeocodingRequest [query='" + _query + "']";
    }
    
}

#endif
