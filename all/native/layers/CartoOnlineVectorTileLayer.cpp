#include "CartoOnlineVectorTileLayer.h"
#include "core/BinaryData.h"
#include "datasources/CartoOnlineTileDataSource.h"
#include "styles/CompiledStyleSet.h"
#include "vectortiles/MBVectorTileDecoder.h"
#include "utils/ZippedAssetPackage.h"
#include "utils/AssetUtils.h"
#include "utils/Log.h"

namespace carto {
    
    CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(const std::string& styleAssetName) :
        VectorTileLayer(CreateDataSource(DEFAULT_SOURCE), CreateTileDecoder(styleAssetName))
    {
    }
    
    CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(const std::string& source, const std::string& styleAssetName) :
        VectorTileLayer(CreateDataSource(source), CreateTileDecoder(styleAssetName))
    {
    }
    
    CartoOnlineVectorTileLayer::~CartoOnlineVectorTileLayer() {
    }
    
    std::shared_ptr<TileDataSource> CartoOnlineVectorTileLayer::CreateDataSource(const std::string& source) {
        return std::make_shared<CartoOnlineTileDataSource>(source);
    }
    
    std::shared_ptr<VectorTileDecoder> CartoOnlineVectorTileLayer::CreateTileDecoder(const std::string& styleAssetName) {
        std::shared_ptr<BinaryData> styleSetData = AssetUtils::LoadAsset(styleAssetName);
        if (!styleSetData) {
            Log::Errorf("CartoOnlineVectorTileLayer: Could not load style asset %s", styleAssetName.c_str());
            styleSetData = std::make_shared<BinaryData>();
        }
        auto styleAssetPackage = std::make_shared<CompiledStyleSet>(std::make_shared<ZippedAssetPackage>(styleSetData));
        return std::make_shared<MBVectorTileDecoder>(styleAssetPackage);
    }
    
    const std::string CartoOnlineVectorTileLayer::DEFAULT_SOURCE = "nutiteq.osm";
}
