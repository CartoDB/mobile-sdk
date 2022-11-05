#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "OSMOfflineReverseGeocodingService.h"
#include "components/Exceptions.h"
#include "geocoding/utils/CartoGeocodingProxy.h"

#include <geocoding/RevGeocoder.h>

#include <sqlite3pp.h>

namespace carto {

    OSMOfflineReverseGeocodingService::OSMOfflineReverseGeocodingService(const std::string& path) :
        _revGeocoder()
    {
        auto database = std::make_shared<sqlite3pp::database>();
        if (database->connect_v2(path.c_str(), SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX) != SQLITE_OK) {
            throw FileException("Failed to open geocoding database", path);
        }
        database->execute("PRAGMA temp_store=MEMORY");

        _revGeocoder = std::make_shared<geocoding::RevGeocoder>();
        if (!_revGeocoder->import(database)) {
            throw GenericException("Failed to import geocoding database", path);
        }
    }

    OSMOfflineReverseGeocodingService::~OSMOfflineReverseGeocodingService() {
    }

    std::string OSMOfflineReverseGeocodingService::getLanguage() const {
        return _revGeocoder->getLanguage();
    }

    void OSMOfflineReverseGeocodingService::setLanguage(const std::string& lang) {
        _revGeocoder->setLanguage(lang);
    }

    std::vector<std::shared_ptr<GeocodingResult> > OSMOfflineReverseGeocodingService::calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        return CartoGeocodingProxy::CalculateAddresses(_revGeocoder, request);
    }
    
}

#endif
