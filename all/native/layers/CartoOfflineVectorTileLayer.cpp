#include "CartoOfflineVectorTileLayer.h"
#include "datasources/PackageManagerTileDataSource.h"
#include "packagemanager/CartoPackageManager.h"
#include "utils/AssetPackage.h"
#include "utils/Log.h"

namespace carto {
    
    CartoOfflineVectorTileLayer::CartoOfflineVectorTileLayer(const std::shared_ptr<CartoPackageManager>& packageManager, CartoBaseMapStyle::CartoBaseMapStyle style) :
        CartoVectorTileLayer(std::make_shared<PackageManagerTileDataSource>(packageManager), style)
    {
    }

    CartoOfflineVectorTileLayer::CartoOfflineVectorTileLayer(const std::shared_ptr<CartoPackageManager>& packageManager, const std::shared_ptr<AssetPackage>& styleAssetPackage) :
        CartoVectorTileLayer(std::make_shared<PackageManagerTileDataSource>(packageManager), styleAssetPackage)
    {
    }
    
    CartoOfflineVectorTileLayer::~CartoOfflineVectorTileLayer() {
    }
    
}
