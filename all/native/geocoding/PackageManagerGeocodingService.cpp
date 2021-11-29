#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "PackageManagerGeocodingService.h"
#include "components/Exceptions.h"
#include "geocoding/utils/CartoGeocodingProxy.h"
#include "packagemanager/PackageInfo.h"
#include "packagemanager/handlers/GeocodingPackageHandler.h"

#include <geocoding/Geocoder.h>

#include <sqlite3pp.h>

namespace carto {

    PackageManagerGeocodingService::PackageManagerGeocodingService(const std::shared_ptr<PackageManager>& packageManager) :
        _packageManager(packageManager),
        _autocomplete(false),
        _language(),
        _maxResults(10),
        _cachedPackageDatabaseMap(),
        _cachedGeocoder(),
        _mutex()
    {
        if (!packageManager) {
            throw NullArgumentException("Null packageManager");
        }

        _packageManagerListener = std::make_shared<PackageManagerListener>(*this);
        _packageManager->registerOnChangeListener(_packageManagerListener);
    }

    PackageManagerGeocodingService::~PackageManagerGeocodingService() {
        _packageManager->unregisterOnChangeListener(_packageManagerListener);
        _packageManagerListener.reset();
    }

    bool PackageManagerGeocodingService::isAutocomplete() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _autocomplete;
    }

    void PackageManagerGeocodingService::setAutocomplete(bool autocomplete) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (autocomplete != _autocomplete) {
            _autocomplete = autocomplete;
            _cachedGeocoder.reset();
        }
    }

    std::string PackageManagerGeocodingService::getLanguage() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _language;
    }

    void PackageManagerGeocodingService::setLanguage(const std::string& lang) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (lang != _language) {
            _language = lang;
            _cachedGeocoder.reset();
        }
    }

    int PackageManagerGeocodingService::getMaxResults() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _maxResults;
    }

    void PackageManagerGeocodingService::setMaxResults(int maxResults) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (maxResults != _maxResults) {
            _maxResults = maxResults;
            _cachedGeocoder.reset();
        }
    }

    std::vector<std::shared_ptr<GeocodingResult> > PackageManagerGeocodingService::calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        // Do routing via package manager, so that all packages are locked during routing
        std::vector<std::shared_ptr<GeocodingResult> > results;
        _packageManager->accessLocalPackages([this, &results, &request](const std::map<std::shared_ptr<PackageInfo>, std::shared_ptr<PackageHandler> >& packageHandlerMap) {
            // Build map of geocoding databases
            std::map<std::shared_ptr<PackageInfo>, std::shared_ptr<sqlite3pp::database> > packageDatabaseMap;
            for (auto it = packageHandlerMap.begin(); it != packageHandlerMap.end(); it++) {
                if (auto geocodingHandler = std::dynamic_pointer_cast<GeocodingPackageHandler>(it->second)) {
                    if (auto packageDatabase = geocodingHandler->getDatabase()) {
                        packageDatabaseMap[it->first] = packageDatabase;
                    }
                }
            }

            // Now check if we have to reinitialize the geocoder
            std::lock_guard<std::mutex> lock(_mutex);
            if (!_cachedGeocoder || packageDatabaseMap != _cachedPackageDatabaseMap) {
                auto geocoder = std::make_shared<geocoding::Geocoder>();
                geocoder->setAutocomplete(_autocomplete);
                geocoder->setLanguage(_language);
                geocoder->setMaxResults(_maxResults);
                for (auto it = packageDatabaseMap.begin(); it != packageDatabaseMap.end(); it++) {
                    try {
                        if (!geocoder->import(it->second)) {
                            throw FileException("Failed to import geocoding database " + it->first->getPackageId(), "");
                        }
                    }
                    catch (const std::exception& ex) {
                        throw GenericException("Exception while importing geocoding database " + it->first->getPackageId(), ex.what());
                    }
                }
                _cachedPackageDatabaseMap = packageDatabaseMap;
                _cachedGeocoder = geocoder;
            }

            results = CartoGeocodingProxy::CalculateAddresses(_cachedGeocoder, request);
        });
        return results;
    }
    
    PackageManagerGeocodingService::PackageManagerListener::PackageManagerListener(PackageManagerGeocodingService& service) :
        _service(service)
    {
    }
        
    void PackageManagerGeocodingService::PackageManagerListener::onPackagesChanged(PackageChangeType changeType) {
        std::lock_guard<std::mutex> lock(_service._mutex);
        _service._cachedPackageDatabaseMap.clear();
        _service._cachedGeocoder.reset();
    }

    void PackageManagerGeocodingService::PackageManagerListener::onStylesChanged() {
        // Impossible
    }

}

#endif
