#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "CartoOfflineVectorTileLayer.h"
#include "components/Exceptions.h"
#include "datasources/PackageManagerTileDataSource.h"
#include "packagemanager/CartoPackageManager.h"
#include "styles/CompiledStyleSet.h"
#include "vectortiles/MBVectorTileDecoder.h"
#include "utils/AssetPackage.h"
#include "utils/Log.h"

namespace carto {
    
    CartoOfflineVectorTileLayer::CartoOfflineVectorTileLayer(const std::shared_ptr<CartoPackageManager>& packageManager, CartoBaseMapStyle::CartoBaseMapStyle style) :
        CartoVectorTileLayer(std::make_shared<PackageManagerTileDataSource>(packageManager), packageManager ? packageManager->getStyleAssetPackage(style) : std::shared_ptr<AssetPackage>(), GetStyleName(style)),
        _style(style),
        _packageManager(packageManager),
        _packageManagerListener()
    {
        if (!packageManager) {
            throw NullArgumentException("Null packagemanager");
        }

        setPreloading(true); // turn preloading on by default

        _packageManagerListener = std::make_shared<PackageManagerListener>(*this);
        _packageManager->registerOnChangeListener(_packageManagerListener);
    }

    CartoOfflineVectorTileLayer::CartoOfflineVectorTileLayer(const std::shared_ptr<CartoPackageManager>& packageManager, const std::shared_ptr<AssetPackage>& styleAssetPackage) :
        CartoVectorTileLayer(std::make_shared<PackageManagerTileDataSource>(packageManager), styleAssetPackage),
        _style(),
        _packageManager(packageManager),
        _packageManagerListener()
    {
        if (!packageManager) {
            throw NullArgumentException("Null packagemanager");
        }

        setPreloading(true); // turn preloading on by default
    }
    
    CartoOfflineVectorTileLayer::~CartoOfflineVectorTileLayer() {
        if (_packageManagerListener) {
            _packageManager->unregisterOnChangeListener(_packageManagerListener);
            _packageManagerListener.reset();
        }
    }
    
    CartoOfflineVectorTileLayer::PackageManagerListener::PackageManagerListener(CartoOfflineVectorTileLayer& layer) :
        _layer(layer)
    {
    }
        
    void CartoOfflineVectorTileLayer::PackageManagerListener::onPackagesChanged() {
        // NOTE: already handled by PackageManagerTileDataSource
    }
    
    void CartoOfflineVectorTileLayer::PackageManagerListener::onStylesChanged() {
        std::shared_ptr<AssetPackage> updatedAssetPackage = _layer._packageManager->getStyleAssetPackage(_layer._style);
        if (auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(_layer.getTileDecoder())) {
            tileDecoder->setCompiledStyleSet(std::make_shared<CompiledStyleSet>(updatedAssetPackage, GetStyleName(_layer._style)));
        }
    }

}

#endif
