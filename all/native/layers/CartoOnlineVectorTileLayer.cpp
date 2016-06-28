#include "CartoOnlineVectorTileLayer.h"
#include "core/BinaryData.h"
#include "datasources/CartoOnlineTileDataSource.h"
#include "styles/CompiledStyleSet.h"
#include "vectortiles/MBVectorTileDecoder.h"
#include "utils/AssetPackage.h"
#include "utils/Log.h"

namespace carto {
    
    CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(const std::string& source, const std::shared_ptr<AssetPackage>& styleAssetPackage) :
        VectorTileLayer(CreateDataSource(source), CreateTileDecoder(styleAssetPackage))
    {
    }
    
    CartoOnlineVectorTileLayer::~CartoOnlineVectorTileLayer() {
    }
    
    std::shared_ptr<TileDataSource> CartoOnlineVectorTileLayer::CreateDataSource(const std::string& source) {
        return std::make_shared<CartoOnlineTileDataSource>(source);
    }
    
    std::shared_ptr<VectorTileDecoder> CartoOnlineVectorTileLayer::CreateTileDecoder(const std::shared_ptr<AssetPackage>& styleAssetPackage) {
        if (!styleAssetPackage) {
            Log::Error("CartoOnlineVectorTileLayer: Null asset package!");
            return std::shared_ptr<VectorTileDecoder>();
        }
        return std::make_shared<MBVectorTileDecoder>(std::make_shared<CompiledStyleSet>(styleAssetPackage));
    }
    
}
