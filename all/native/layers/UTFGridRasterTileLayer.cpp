#include "UTFGridRasterTileLayer.h"
#include "core/BinaryData.h"
#include "components/CancelableThreadPool.h"
#include "datasources/TileDataSource.h"
#include "renderers/MapRenderer.h"
#include "utils/Utils.h"
#include "utils/Log.h"
#include "vectortiles/VectorTileDecoder.h"
#include "vt/TileId.h"
#include "vt/Tile.h"

#include <boost/lexical_cast.hpp>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <utf8.h>

namespace carto {

    UTFGridRasterTileLayer::UTFGridRasterTileLayer(const std::shared_ptr<TileDataSource>& rasterDataSource, const std::shared_ptr<TileDataSource>& utfGridDataSource) :
        RasterTileLayer(rasterDataSource),
        _utfGridDataSource(utfGridDataSource),
        _utfGridVisibleCache(64 * 1024 * 1024), // NOTE: the limit should never be reached in normal cases
        _utfGridPreloadingCache(DEFAULT_PRELOADING_CACHE_SIZE)
    {
    }
    
    UTFGridRasterTileLayer::~UTFGridRasterTileLayer() {
    }
    
    void UTFGridRasterTileLayer::clearTileCaches(bool all) {
        RasterTileLayer::clearTileCaches(all);
        if (all) {
            _utfGridVisibleCache.clear();
        }
        _utfGridPreloadingCache.clear();
    }
    
    std::map<std::string, std::string> UTFGridRasterTileLayer::getTooltips(const MapPos& mapPos, bool waitForData) const {
        int zoom = getMaxZoom();
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            if (_lastCullState) {
                zoom = std::min(getMaxZoom(), static_cast<int>(_lastCullState->getViewState().getZoom() + getZoomLevelBias() + DISCRETE_ZOOM_LEVEL_BIAS));
            }
        }

        MapTile mapTile = calculateMapTile(mapPos, std::min(_utfGridDataSource->getMaxZoom(), std::max(_utfGridDataSource->getMinZoom(), zoom)));
        double tileWidth = _utfGridDataSource->getProjection()->getBounds().getDelta().getX() / (1 << mapTile.getZoom());
        double tileHeight = _utfGridDataSource->getProjection()->getBounds().getDelta().getY() / (1 << mapTile.getZoom());
        MapVec mapVec(mapTile.getX() * tileWidth, mapTile.getY() * tileHeight);
        MapPos mapTileOrigin = _utfGridDataSource->getProjection()->getBounds().getMin() + mapVec;
        double xRel = (mapPos.getX() - mapTileOrigin.getX()) / tileWidth;
        double yRel = 1 - (mapPos.getY() - mapTileOrigin.getY()) / tileHeight;
        long long tileId = mapTile.getTileId();
        
        // Try to get the tile from cache
        std::shared_ptr<UTFGridTile> tile;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            if (!_utfGridVisibleCache.read(tileId, tile)) {
                _utfGridPreloadingCache.read(tileId, tile);
            }
        }
        if (!tile) {
            if (waitForData) {
                // Fetch the tile
                CombinedFetchTask(std::const_pointer_cast<UTFGridRasterTileLayer>(std::static_pointer_cast<const UTFGridRasterTileLayer>(shared_from_this())), mapTile, false)();

                std::lock_guard<std::recursive_mutex> lock(_mutex);
                _utfGridVisibleCache.read(tileId, tile);
            }
        }

        if (tile) {
            int x = static_cast<int>(std::floor(xRel * tile->getXSize()));
            int y = static_cast<int>(std::floor(yRel * tile->getYSize()));
            int keyId = tile->getKeyId(x, y);
            if (keyId != 0) {
                std::string key = tile->getKey(keyId);
                return tile->getData(key);
            }
        }
        return std::map<std::string, std::string>();
    }
        
    bool UTFGridRasterTileLayer::tileExists(const MapTile& tile, bool preloadingCache) {
        if (!RasterTileLayer::tileExists(tile, preloadingCache)) {
            return false;
        }
        if (tile.getZoom() < _utfGridDataSource->getMinZoom() || tile.getZoom() > _utfGridDataSource->getMaxZoom()) {
            return true;
        }

        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (preloadingCache) {
            return _utfGridPreloadingCache.exists(tile.getTileId());
        }
        return _utfGridVisibleCache.exists(tile.getTileId());
    }
    
    bool UTFGridRasterTileLayer::tileIsValid(const MapTile& tile) const {
        if (!RasterTileLayer::tileIsValid(tile)) {
            return false;
        }
        if (tile.getZoom() < _utfGridDataSource->getMinZoom() || tile.getZoom() > _utfGridDataSource->getMaxZoom()) {
            return true;
        }

        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (_utfGridVisibleCache.exists(tile.getTileId())) {
            return _utfGridVisibleCache.valid(tile.getTileId());
        }
        if (_utfGridPreloadingCache.exists(tile.getTileId())) {
            return _utfGridPreloadingCache.valid(tile.getTileId());
        }
        return false;
    }
    
    void UTFGridRasterTileLayer::fetchTile(const MapTile& tile, bool preloadingTile, bool invalidated) {
        if (!invalidated) {
            if (tileExists(tile, preloadingTile)) {
                return;
            }
        }
        
        auto task = std::make_shared<CombinedFetchTask>(std::static_pointer_cast<UTFGridRasterTileLayer>(shared_from_this()), tile, preloadingTile);
        _fetchingTiles.add(tile.getTileId(), task);
        
        std::shared_ptr<CancelableThreadPool> tileThreadPool;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            tileThreadPool = _tileThreadPool;
        }
        if (tileThreadPool) {
            tileThreadPool->execute(task, preloadingTile ? getUpdatePriority() + PRELOADING_PRIORITY_OFFSET : getUpdatePriority());
        }
    }
    
    void UTFGridRasterTileLayer::tilesChanged(bool removeTiles) {
        // Flush caches
        if (removeTiles) {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _utfGridVisibleCache.clear();
            _utfGridPreloadingCache.clear();
        } else {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _utfGridVisibleCache.clear();
            _utfGridPreloadingCache.clear();
        }
        RasterTileLayer::tilesChanged(removeTiles);
    }
    
    void UTFGridRasterTileLayer::registerDataSourceListener() {
        RasterTileLayer::registerDataSourceListener();
        _utfGridDataSource->registerOnChangeListener(_dataSourceListener);
    }
    
    void UTFGridRasterTileLayer::unregisterDataSourceListener() {
        _utfGridDataSource->unregisterOnChangeListener(_dataSourceListener);
        RasterTileLayer::unregisterDataSourceListener();
    }
    
    std::shared_ptr<UTFGridRasterTileLayer::UTFGridTile> UTFGridRasterTileLayer::DecodeUTFTile(const TileData& tileData) {
        // TODO: check if inflated
        std::string json(reinterpret_cast<const char*>(tileData.getData()->data()), tileData.getData()->size());
        rapidjson::Document doc;
        if (doc.Parse<rapidjson::kParseDefaultFlags>(json.c_str()).HasParseError()) {
            Log::Error("UTFGridRasterTileLayer::DecodeUTFTile: Failed to parse JSON");
            return std::shared_ptr<UTFGridTile>();
        }
        
        std::vector<std::string> keys;
        for (unsigned int i = 0; i < doc["keys"].Size(); i++) {
            keys.push_back(doc["keys"][i].GetString());
        }
        
        std::map<std::string, std::map<std::string, std::string> > data;
        if (doc.FindMember("data") != doc.MemberEnd()) {
            for (rapidjson::Value::ConstMemberIterator it = doc["data"].MemberBegin(); it != doc["data"].MemberEnd(); ++it) {
                if (!it->name.IsString() || !it->value.IsObject()) {
                    continue;
                }
                std::string key = it->name.GetString();
                for (rapidjson::Value::ConstMemberIterator it2 = it->value.MemberBegin(); it2 != it->value.MemberEnd(); it2++) {
                    const rapidjson::Value* value = &it2->value;
                    if (!it2->name.IsString()) {
                        continue;
                    }
                    std::string str;
                    if (value->IsString()) {
                        str = value->GetString();
                    }
                    else if (value->IsInt()) {
                        str = boost::lexical_cast<std::string>(value->GetInt());
                    }
                    else if (value->IsUint()) {
                        str = boost::lexical_cast<std::string>(value->GetUint());
                    }
                    else if (value->IsInt64()) {
                        str = boost::lexical_cast<std::string>(value->GetInt64());
                    }
                    else if (value->IsUint64()) {
                        str = boost::lexical_cast<std::string>(value->GetUint64());
                    }
                    else if (value->IsNumber()) {
                        str = boost::lexical_cast<std::string>(value->GetDouble());
                    }
                    data[key][it2->name.GetString()] = str;
                }
            }
        }
        
        unsigned int rows = doc["grid"].Size();
        unsigned int cols = 0;
        for (unsigned int i = 0; i < rows; i++) {
            std::string columnUTF8 = doc["grid"][i].GetString();
            std::vector<std::uint32_t> column;
            column.reserve(columnUTF8.size());
            utf8::utf8to32(columnUTF8.begin(), columnUTF8.end(), std::back_inserter(column));

            cols = std::max(cols, static_cast<unsigned int>(column.size()));
        }
        std::vector<int> keyIds;
        keyIds.reserve(cols * rows);
        for (unsigned int i = 0; i < rows; i++) {
            std::string columnUTF8 = doc["grid"][i].GetString();
            std::vector<std::uint32_t> column;
            column.reserve(columnUTF8.size());
            utf8::utf8to32(columnUTF8.begin(), columnUTF8.end(), std::back_inserter(column));

            if (column.size() != cols) {
                Log::Warnf("UTFGridRasterTileLayer::DecodeUTFTile: Mismatching rows/columns");
                column.resize(cols - column.size(), ' ');
            }
            
            for (std::size_t j = 0; j < column.size(); j++) {
                std::uint32_t code = column[j];
                if (code >= 93) code--;
                if (code >= 35) code--;
                code -= 32;
                keyIds.push_back(code);
            }
        }
        return std::make_shared<UTFGridTile>(keys, data, keyIds, cols, rows);
    }
    
    UTFGridRasterTileLayer::CombinedFetchTask::CombinedFetchTask(const std::shared_ptr<UTFGridRasterTileLayer>& layer, const MapTile& tile, bool preloadingTile) :
        FetchTask(layer, tile, preloadingTile)
    {
    }
    
    bool UTFGridRasterTileLayer::CombinedFetchTask::loadTile(const std::shared_ptr<TileLayer>& tileLayer) {
        bool refresh = FetchTask::loadTile(tileLayer);

        auto layer = std::static_pointer_cast<UTFGridRasterTileLayer>(tileLayer);
        for (const MapTile& dataSourceTile : _dataSourceTiles) {
            if (dataSourceTile.getZoom() < layer->_utfGridDataSource->getMinZoom() || dataSourceTile.getZoom() > layer->_utfGridDataSource->getMaxZoom()) {
                break;
            }

            std::shared_ptr<UTFGridRasterTileLayer::UTFGridTile> utfTile;

            std::shared_ptr<TileData> tileData = layer->_utfGridDataSource->loadTile(dataSourceTile);
            if (tileData) {
                utfTile = UTFGridRasterTileLayer::DecodeUTFTile(*tileData);
                if (!utfTile) {
                    Log::Error("UTFGridRasterTileLayer::CombinedFetchTask: Failed to decode tile");
                }
            }

            if (utfTile) {
                // Store tile to cache, unless invalidated
                if (!isInvalidated()) {
                    unsigned int tileSize = static_cast<unsigned int>(utfTile->getResidentSize());
                    if (isPreloading()) {
                        std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                        layer->_utfGridPreloadingCache.put(_tile.getTileId(), utfTile, tileSize);
                        if (tileData->getMaxAge() >= 0) {
                            layer->_utfGridPreloadingCache.invalidate(_tile.getTileId(), std::chrono::steady_clock::now() + std::chrono::milliseconds(tileData->getMaxAge()));
                        }
                    }
                    else {
                        std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                        layer->_utfGridVisibleCache.put(_tile.getTileId(), utfTile, tileSize);
                        if (tileData->getMaxAge() >= 0) {
                            layer->_utfGridVisibleCache.invalidate(_tile.getTileId(), std::chrono::steady_clock::now() + std::chrono::milliseconds(tileData->getMaxAge()));
                        }
                    }
                }
                refresh = true; // NOTE: need to refresh even when invalidated
            } else {
                // Store null tile to the cache
                std::lock_guard<std::mutex> lock(_mutex);
                if (!isInvalidated()) {
                    std::size_t tileSize = 1024;
                    if (isPreloading()) {
                        std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                        layer->_utfGridPreloadingCache.put(_tile.getTileId(), utfTile, tileSize);
                    } else {
                        std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                        layer->_utfGridVisibleCache.put(_tile.getTileId(), utfTile, tileSize);
                    }
        }
                break;
            }
            break;
        }
        
        return refresh;
    }
        
}
