#include "CartoOfflineVectorTileLayer.h"
#include "datasources/PackageManagerTileDataSource.h"
#include "packagemanager/CartoPackageManager.h"
#include "utils/AssetPackage.h"
#include "utils/Log.h"

namespace carto {
    
    CartoOfflineVectorTileLayer::CartoOfflineVectorTileLayer(const std::shared_ptr<CartoPackageManager>& packageManager, CartoBaseMapStyle::CartoBaseMapStyle style) :
        CartoVectorTileLayer(std::make_shared<PackageManagerTileDataSource>(packageManager), style),
        _packageManager(packageManager),
        _packageManagerListener()
    {
        _preloading = true; // turn preloading on by default

        _packageManagerListener = std::make_shared<PackageManagerListener>(*this);
        _packageManager->registerOnChangeListener(_packageManagerListener);
    }

    CartoOfflineVectorTileLayer::CartoOfflineVectorTileLayer(const std::shared_ptr<CartoPackageManager>& packageManager, const std::shared_ptr<AssetPackage>& styleAssetPackage) :
        CartoVectorTileLayer(std::make_shared<PackageManagerTileDataSource>(packageManager), styleAssetPackage),
        _packageManager(packageManager),
        _packageManagerListener()
    {
        _preloading = true; // turn preloading on by default

        _packageManagerListener = std::make_shared<PackageManagerListener>(*this);
        _packageManager->registerOnChangeListener(_packageManagerListener);
    }
    
    CartoOfflineVectorTileLayer::~CartoOfflineVectorTileLayer() {
        _packageManager->unregisterOnChangeListener(_packageManagerListener);
        _packageManagerListener.reset();
    }
    
    CartoOfflineVectorTileLayer::PackageManagerListener::PackageManagerListener(CartoOfflineVectorTileLayer& layer) :
        _layer(layer)
    {
    }
        
    void CartoOfflineVectorTileLayer::PackageManagerListener::onPackagesChanged() {
        // NOTE: already handled by PackageManagerTileDataSource
    }
    
    void CartoOfflineVectorTileLayer::PackageManagerListener::onStylesChanged() {
        _layer.tilesChanged(false);
    }

}
