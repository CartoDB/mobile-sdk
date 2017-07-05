#include "CartoOnlineVectorTileLayer.h"
#include "datasources/CartoOnlineTileDataSource.h"
#include "utils/AssetPackage.h"
#include "utils/Log.h"

namespace carto {
    
    CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(CartoBaseMapStyle::CartoBaseMapStyle style) :
        CartoVectorTileLayer(CreateDataSource(style), style)
    {
    }

    CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(const std::string& source, CartoBaseMapStyle::CartoBaseMapStyle style) :
        CartoVectorTileLayer(std::make_shared<CartoOnlineTileDataSource>(source), style)
    {
    }

    CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(const std::string& source, const std::shared_ptr<AssetPackage>& styleAssetPackage) :
        CartoVectorTileLayer(std::make_shared<CartoOnlineTileDataSource>(source), styleAssetPackage)
    {
    }
    
    CartoOnlineVectorTileLayer::~CartoOnlineVectorTileLayer() {
    }

    std::shared_ptr<TileDataSource> CartoOnlineVectorTileLayer::CreateDataSource(CartoBaseMapStyle::CartoBaseMapStyle style) {
        switch (style) {
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_VOYAGER:
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_POSITRON:
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_DARKMATTER:
            return std::make_shared<CartoOnlineTileDataSource>("carto-omt.osm");
        default:
            return std::make_shared<CartoOnlineTileDataSource>("nutiteq.osm");
        }
    }
    
}
