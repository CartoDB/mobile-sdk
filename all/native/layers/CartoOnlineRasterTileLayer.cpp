#include "CartoOnlineRasterTileLayer.h"
#include "components/Exceptions.h"
#include "datasources/CartoOnlineTileDataSource.h"
#include "utils/Log.h"

namespace carto {
    
    CartoOnlineRasterTileLayer::CartoOnlineRasterTileLayer(const std::string& source) :
        RasterTileLayer(CreateDataSource(source))
    {
    }
    
    CartoOnlineRasterTileLayer::~CartoOnlineRasterTileLayer() {
    }
    
    std::shared_ptr<TileDataSource> CartoOnlineRasterTileLayer::CreateDataSource(const std::string& source) {
        return std::make_shared<CartoOnlineTileDataSource>(source);
    }
    
}
