#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "PackageManagerReverseGeocodingService.h"
#include "components/Exceptions.h"
#include "geocoding/GeocodingProxy.h"
#include "packagemanager/PackageInfo.h"
#include "packagemanager/handlers/GeocodingPackageHandler.h"

#include <geocoding/RevGeocoder.h>

#include <sqlite3pp.h>

namespace carto {

    PackageManagerReverseGeocodingService::PackageManagerReverseGeocodingService(const std::shared_ptr<PackageManager>& packageManager) :
        _packageManager(packageManager),
        _searchRadius(100.0f),
        _cachedPackageDatabaseMap(),
        _cachedRevGeocoder(),
        _mutex()
    {
        if (!packageManager) {
            throw NullArgumentException("Null packageManager");
        }

        _packageManagerListener = std::make_shared<PackageManagerListener>(*this);
        _packageManager->registerOnChangeListener(_packageManagerListener);
    }

    PackageManagerReverseGeocodingService::~PackageManagerReverseGeocodingService() {
        _packageManager->unregisterOnChangeListener(_packageManagerListener);
        _packageManagerListener.reset();
    }

    float PackageManagerReverseGeocodingService::getSearchRadius() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _searchRadius;
    }

    void PackageManagerReverseGeocodingService::setSearchRadius(float radius) {
        std::lock_guard<std::mutex> lock(_mutex);
        _searchRadius = radius;
        _cachedRevGeocoder.reset();
    }

    std::vector<std::shared_ptr<GeocodingResult> > PackageManagerReverseGeocodingService::calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const {
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
                    packageDatabaseMap[it->first] = geocodingHandler->getGeocodingDatabase();
                }
            }

            // Now check if we have to reinitialize the geocoder
            std::lock_guard<std::mutex> lock(_mutex);
            if (!_cachedRevGeocoder || packageDatabaseMap != _cachedPackageDatabaseMap) {
                auto revGeocoder = std::make_shared<geocoding::RevGeocoder>();
                revGeocoder->setRadius(_searchRadius);
                for (auto it = packageDatabaseMap.begin(); it != packageDatabaseMap.end(); it++) {
                    try {
                        if (!revGeocoder->import(it->second)) {
                            throw FileException("Failed to import geocoding database " + it->first->getPackageId(), "");
                        }
                    }
                    catch (const std::exception& ex) {
                        throw GenericException("Exception while importing geocoding database " + it->first->getPackageId(), ex.what());
                    }
                }
                _cachedPackageDatabaseMap = packageDatabaseMap;
                _cachedRevGeocoder = revGeocoder;
            }

            results = GeocodingProxy::CalculateAddresses(_cachedRevGeocoder, request);
        });
        return results;
    }
    
    PackageManagerReverseGeocodingService::PackageManagerListener::PackageManagerListener(PackageManagerReverseGeocodingService& service) :
        _service(service)
    {
    }
        
    void PackageManagerReverseGeocodingService::PackageManagerListener::onPackagesChanged() {
        std::lock_guard<std::mutex> lock(_service._mutex);
        _service._cachedPackageDatabaseMap.clear();
        _service._cachedRevGeocoder.reset();
    }

}

#endif
