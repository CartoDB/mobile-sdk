#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "OSMOfflineGeocodingService.h"
#include "components/Exceptions.h"
#include "geocoding/GeocodingProxy.h"

#include <geocoding/Geocoder.h>

#include <sqlite3pp.h>

namespace carto {

    OSMOfflineGeocodingService::OSMOfflineGeocodingService(const std::string& path) :
        _database(),
        _geocoder()
    {
        try {
            _database = std::make_shared<sqlite3pp::database>(path.c_str());
            _geocoder = std::make_shared<geocoding::Geocoder>(*_database);
        } catch (const std::exception& ex) {
            throw FileException("Failed to import geocoding database", path);
        }
    }

    OSMOfflineGeocodingService::~OSMOfflineGeocodingService() {
    }

    bool OSMOfflineGeocodingService::isAutocomplete() const {
        return _geocoder->getAutocomplete();
    }

    void OSMOfflineGeocodingService::setAutocomplete(bool autocomplete) {
        return _geocoder->setAutocomplete(autocomplete);
    }

    std::vector<std::shared_ptr<GeocodingResult> > OSMOfflineGeocodingService::calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        return GeocodingProxy::CalculateAddresses(_geocoder, request);
    }
    
}

#endif
