#include "CartoOnlineVectorTileLayer.h"
#include "datasources/CartoOnlineTileDataSource.h"
#include "utils/AssetPackage.h"
#include "utils/Log.h"

namespace carto {
    
    CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(CartoBaseMapStyle::CartoBaseMapStyle style) :
        CartoVectorTileLayer(std::make_shared<CartoOnlineTileDataSource>("nutiteq.osm"), style)
    {
    }

    CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(const std::string& source, const std::shared_ptr<AssetPackage>& styleAssetPackage) :
        CartoVectorTileLayer(std::make_shared<CartoOnlineTileDataSource>(source), styleAssetPackage)
    {
    }
    
    CartoOnlineVectorTileLayer::~CartoOnlineVectorTileLayer() {
    }
    
}
