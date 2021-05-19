#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "OSMOfflineGeocodingService.h"
#include "components/Exceptions.h"
#include "geocoding/utils/CartoGeocodingProxy.h"

#include <geocoding/Geocoder.h>

#include <sqlite3pp.h>

namespace carto {

    OSMOfflineGeocodingService::OSMOfflineGeocodingService(const std::string& path) :
        _geocoder()
    {
        auto database = std::make_shared<sqlite3pp::database>();
        if (database->connect_v2(path.c_str(), SQLITE_OPEN_READONLY) != SQLITE_OK) {
            throw FileException("Failed to open geocoding database", path);
        }
        database->execute("PRAGMA temp_store=MEMORY");

        _geocoder = std::make_shared<geocoding::Geocoder>();
        if (!_geocoder->import(database)) {
            throw GenericException("Failed to import geocoding database", path);
        }
    }

    OSMOfflineGeocodingService::~OSMOfflineGeocodingService() {
    }

    bool OSMOfflineGeocodingService::isAutocomplete() const {
        return _geocoder->getAutocomplete();
    }

    void OSMOfflineGeocodingService::setAutocomplete(bool autocomplete) {
        _geocoder->setAutocomplete(autocomplete);
    }

    std::string OSMOfflineGeocodingService::getLanguage() const {
        return _geocoder->getLanguage();
    }

    void OSMOfflineGeocodingService::setLanguage(const std::string& lang) {
        _geocoder->setLanguage(lang);
    }

    int OSMOfflineGeocodingService::getMaxResults() const {
        return _geocoder->getMaxResults();
    }

    void OSMOfflineGeocodingService::setMaxResults(int maxResults) {
        _geocoder->setMaxResults(maxResults);
    }

    std::vector<std::shared_ptr<GeocodingResult> > OSMOfflineGeocodingService::calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        return CartoGeocodingProxy::CalculateAddresses(_geocoder, request);
    }
    
}

#endif
