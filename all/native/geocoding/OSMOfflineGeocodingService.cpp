#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "geocoding/OSMOfflineGeocodingService.h"

#include <geocoding/Geocoder.h>

namespace carto {

    OSMOfflineGeocodingService::OSMOfflineGeocodingService(const std::shared_ptr<Projection>& proj) : _projection(proj) {
    }

    OSMOfflineGeocodingService::~OSMOfflineGeocodingService() {
    }

    std::vector<std::shared_ptr<GeocodingResult> > OSMOfflineGeocodingService::calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const {
        return std::vector<std::shared_ptr<GeocodingResult> >();
    }
    
}

#endif
