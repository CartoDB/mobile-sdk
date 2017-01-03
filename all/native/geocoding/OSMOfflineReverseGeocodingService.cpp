#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "geocoding/OSMOfflineReverseGeocodingService.h"

#include <geocoding/RevGeocoder.h>

namespace carto {

    OSMOfflineReverseGeocodingService::OSMOfflineReverseGeocodingService(const std::shared_ptr<Projection>& proj) : _projection(proj) {
    }

    OSMOfflineReverseGeocodingService::~OSMOfflineReverseGeocodingService() {
    }

    std::vector<std::shared_ptr<GeocodingResult> > OSMOfflineReverseGeocodingService::calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const {
        return std::vector<std::shared_ptr<GeocodingResult> >();
    }
    
}

#endif
