#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "OSMOfflineReverseGeocodingService.h"
#include "components/Exceptions.h"
#include "geocoding/GeocodingProxy.h"

#include <geocoding/RevGeocoder.h>

#include <sqlite3pp.h>

namespace carto {

    OSMOfflineReverseGeocodingService::OSMOfflineReverseGeocodingService(const std::string& path) :
        _database(),
        _revGeocoder()
    {
        try {
            _database = std::make_shared<sqlite3pp::database>(path.c_str());
            _revGeocoder = std::make_shared<geocoding::RevGeocoder>(*_database);
        } catch (const std::exception& ex) {
            throw FileException("Failed to import geocoding database", path);
        }
    }

    OSMOfflineReverseGeocodingService::~OSMOfflineReverseGeocodingService() {
    }

    float OSMOfflineReverseGeocodingService::getSearchRadius() const {
        return _revGeocoder->getRadius();
    }

    void OSMOfflineReverseGeocodingService::setSearchRadius(float radius) {
        return _revGeocoder->setRadius(radius);
    }

    std::vector<std::shared_ptr<GeocodingResult> > OSMOfflineReverseGeocodingService::calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        return GeocodingProxy::CalculateAddresses(_revGeocoder, request);
    }
    
}

#endif
