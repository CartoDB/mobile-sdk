#include "AssetTileDataSource.h"
#include "core/MapTile.h"
#include "utils/AssetUtils.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

namespace carto {

    AssetTileDataSource::AssetTileDataSource(int minZoom, int maxZoom, const std::string& basePath) :
        TileDataSource(minZoom, maxZoom),
        _basePath(basePath)
    {
    }
        
    AssetTileDataSource::~AssetTileDataSource(){
    }
    
    std::shared_ptr<TileData> AssetTileDataSource::loadTile(const MapTile& tile) {
        const std::string& path = buildAssetPath(_basePath, tile);
        Log::Infof("AssetTileDataSource::loadTile: Loading %s", path.c_str());
        std::shared_ptr<BinaryData> data = AssetUtils::LoadAsset(path);
        if (!data) {
            Log::Infof("AssetTileDataSource::loadTile: Failed to load %s", path.c_str());
            return std::shared_ptr<TileData>();
        }
        return std::make_shared<TileData>(data);
    }
    
    std::string AssetTileDataSource::buildAssetPath(const std::string& basePath, const MapTile& tile) const {
        std::map<std::string, std::string> tagValues = buildTagValues(tile);
        return GeneralUtils::ReplaceTags(basePath, tagValues, "{", "}", true);
    }
    
}

