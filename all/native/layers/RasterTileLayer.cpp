#include "RasterTileLayer.h"
#include "components/CancelableThreadPool.h"
#include "renderers/MapRenderer.h"
#include "renderers/TileRenderer.h"
#include "renderers/drawdatas/TileDrawData.h"
#include "graphics/Bitmap.h"
#include "utils/Log.h"
#include "vt/TileId.h"
#include "vt/Tile.h"
#include "vt/TileBitmap.h"
#include "vt/TileLayer.h"
#include "vt/TileLayerBuilder.h"

namespace carto {

    RasterTileLayer::RasterTileLayer(const std::shared_ptr<TileDataSource>& dataSource) :
        TileLayer(dataSource),
        _renderer(),
        _tempDrawDatas(),
        _visibleCache(128 * 1024 * 1024), // limit should be never reached during normal use cases
        _preloadingCache(DEFAULT_PRELOADING_CACHE_SIZE)
    {
    }
    
    RasterTileLayer::~RasterTileLayer() {
    }
    
    unsigned int RasterTileLayer::getTextureCacheCapacity() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _preloadingCache.capacity();
    }
    
    void RasterTileLayer::setTextureCacheCapacity(unsigned int capacityInBytes) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _preloadingCache.resize(capacityInBytes);
    }
    
    int RasterTileLayer::getCullDelay() const {
        return CULL_DELAY_TIME;
    }
    
    bool RasterTileLayer::tileExists(const MapTile& tile, bool preloadingCache) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        long long tileId = tile.getTileId();
        if (preloadingCache) {
            return _preloadingCache.exists(tileId);
        }
        else {
            return _visibleCache.exists(tileId);
        }
    }
    
    bool RasterTileLayer::tileValid(const MapTile& tile, bool preloadingCache) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        long long tileId = tile.getTileId();
        if (preloadingCache) {
            return _preloadingCache.exists(tileId) && _preloadingCache.valid(tileId);
        }
        else {
            return _visibleCache.exists(tileId) && _visibleCache.valid(tileId);
        }
    }
    
    void RasterTileLayer::fetchTile(const MapTile& tile, bool preloadingTile, bool invalidated) {
        long long tileId = tile.getTileId();
        if (_fetchingTiles.exists(tile.getTileId())) {
            return;
        }

        if (!invalidated) {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            if (preloadingTile && _preloadingCache.exists(tileId) && _preloadingCache.valid(tileId)) {
                _preloadingCache.get(tileId);
                return;
            }
    
            if (!preloadingTile && _visibleCache.exists(tileId) && _visibleCache.valid(tileId)) {
                _visibleCache.get(tileId);
                return;
            }
        }
    
        auto task = std::make_shared<FetchTask>(std::static_pointer_cast<RasterTileLayer>(shared_from_this()), tile, preloadingTile);
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
    
    void RasterTileLayer::clearTiles(bool preloadingTiles) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (preloadingTiles) {
            _preloadingCache.clear();
        }
        else {
            _visibleCache.clear();
        }
    }

    void RasterTileLayer::tilesChanged(bool removeTiles) {
        // Invalidate current tasks
        for (const std::shared_ptr<FetchTaskBase>& task : _fetchingTiles.getTasks()) {
            task->invalidate();
        }

        // Flush caches
        if (removeTiles) {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _visibleCache.clear();
            _preloadingCache.clear();
        }
        else {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _visibleCache.invalidate_all(std::chrono::steady_clock::now());
            _preloadingCache.invalidate_all(std::chrono::steady_clock::now());
        }
        refresh();
    }

    void RasterTileLayer::calculateDrawData(const MapTile& visTile, const MapTile& closestTile, bool preloadingTile) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        long long closestTileId = closestTile.getTileId();
        std::shared_ptr<const vt::Tile> vtTile;
        _visibleCache.read(closestTileId, vtTile);
        if (!vtTile) {
            _preloadingCache.read(closestTileId, vtTile);
        }
        if (vtTile) {
            vt::TileId vtTileId(visTile.getZoom(), visTile.getX(), visTile.getY());
            if (closestTile.getZoom() > visTile.getZoom()) {
                int dx = visTile.getX() >> visTile.getZoom();
                int dy = visTile.getY() >> visTile.getZoom();
                vtTileId = vt::TileId(closestTile.getZoom(), closestTile.getX() + (dx << closestTile.getZoom()), closestTile.getY() + (dy << closestTile.getZoom()));
            }
            _tempDrawDatas.push_back(std::make_shared<TileDrawData>(vtTileId, vtTile, closestTile.getTileId(), preloadingTile));
        }
    }
    
    void RasterTileLayer::refreshDrawData(const std::shared_ptr<CullState>& cullState) {
        // Move tiles between caches
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            // Get all tiles currently in the visible cache
            std::unordered_set<long long> lastVisibleCacheTiles = _visibleCache.keys();
            
            // Remember unused tiles from the visible cache
            for (const std::shared_ptr<TileDrawData>& drawData : _tempDrawDatas) {
                if (!drawData->isPreloadingTile()) {
                    long long tileId = drawData->getTileId();
                    lastVisibleCacheTiles.erase(tileId);

                    if (!_visibleCache.exists(tileId) && _preloadingCache.exists(tileId)) {
                        _preloadingCache.move(tileId, _visibleCache);
                    }
                }
            }
            
            // Move all unused tiles from visible cache to preloading cache
            for (long long tileId : lastVisibleCacheTiles) {
                _visibleCache.move(tileId, _preloadingCache);
            }
        }
        
        // Update renderer if needed, run culler
        bool refresh = false;
        if (_renderer) {
            if (!(_synchronizedRefresh && _fetchingTiles.getVisibleCount() > 0)) {
                if (_renderer->refreshTiles(_tempDrawDatas)) {
                    refresh = true;
                }
            }
        }
    
        if (refresh) {
            if (std::shared_ptr<MapRenderer> mapRenderer = _mapRenderer.lock()) {
                mapRenderer->requestRedraw();
            }
        }
        
        _tempDrawDatas.clear();
    }
    
    int RasterTileLayer::getMinZoom() const {
        return _dataSource->getMinZoom();
    }
    
    int RasterTileLayer::getMaxZoom() const {
        return _dataSource->getMaxZoom();
    }
        
    void RasterTileLayer::offsetLayerHorizontally(double offset) {
        _renderer->offsetLayerHorizontally(offset);
    }
    
    void RasterTileLayer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        Layer::onSurfaceCreated(shaderManager, textureManager);

        if (_renderer) {
            _renderer->onSurfaceDestroyed();
            _renderer.reset();
    
            // Clear all tile caches - renderer may cache/release tile info, so old tiles are potentially unusable at this point
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _preloadingCache.clear();
            _visibleCache.clear();
        }
    
        // Create new rendererer, simply drop old one (if exists)
        _renderer = std::make_shared<TileRenderer>(_mapRenderer, false, false, false);
        _renderer->onSurfaceCreated(shaderManager, textureManager);
    }
    
    bool RasterTileLayer::onDrawFrame(float deltaSeconds, BillboardSorter& BillboardSorter, StyleTextureCache& styleCache, const ViewState& viewState) {
        updateTileLoadListener();

        return _renderer->onDrawFrame(deltaSeconds, viewState);
    }
    
    bool RasterTileLayer::onDrawFrame3D(float deltaSeconds, BillboardSorter& BillboardSorter, StyleTextureCache& styleCache, const ViewState& viewState) {
        return _renderer->onDrawFrame3D(deltaSeconds, viewState);
    }

    void RasterTileLayer::onSurfaceDestroyed() {
        _renderer->onSurfaceDestroyed();
        _renderer.reset();
        
        // Clear all tile caches - renderer may cache/release tile info, so old tiles are potentially unusable at this point
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _preloadingCache.clear();
            _visibleCache.clear();
        }
    
        Layer::onSurfaceDestroyed();
    }
    
    void RasterTileLayer::registerDataSourceListener() {
        _dataSourceListener = std::make_shared<DataSourceListener>(std::static_pointer_cast<RasterTileLayer>(shared_from_this()));
        _dataSource->registerOnChangeListener(_dataSourceListener);
    }
    
    void RasterTileLayer::unregisterDataSourceListener() {
        _dataSource->unregisterOnChangeListener(_dataSourceListener);
        _dataSourceListener.reset();
    }
    
    RasterTileLayer::FetchTask::FetchTask(const std::shared_ptr<RasterTileLayer>& layer, const MapTile& tile, bool preloadingTile) :
        FetchTaskBase(layer, tile, preloadingTile)
    {
    }
    
    bool RasterTileLayer::FetchTask::loadTile(const std::shared_ptr<TileLayer>& tileLayer) {
        auto layer = std::static_pointer_cast<RasterTileLayer>(tileLayer);
    
        bool refresh = false;
        for (const MapTile& dataSourceTile : _dataSourceTiles) {
            std::shared_ptr<TileData> tileData = layer->_dataSource->loadTile(dataSourceTile);
            if (!tileData) {
                break;
            }
            if (tileData->isReplaceWithParent()) {
                continue;
            }
    
            // Save tile to texture cache, unless invalidated
            vt::TileId vtTile(_tile.getZoom(), _tile.getX(), _tile.getY());
            vt::TileId vtDataSourceTile(dataSourceTile.getZoom(), dataSourceTile.getX(), dataSourceTile.getY());
            std::shared_ptr<Bitmap> bitmap = Bitmap::CreateFromCompressed(tileData->getData());
            if (bitmap) {
                // Check if we received the requested tile or extract/scale the corresponding part
                if (dataSourceTile != _tile) {
                    bitmap = extractSubTile(_tile, dataSourceTile, bitmap);
                }

                if (!isInvalidated()) {
                    // Build the bitmap object
                    std::shared_ptr<vt::Tile> vtTile = createVectorTile(_tile, bitmap);
                    unsigned int tileSize = EXTRA_TILE_FOOTPRINT + vtTile->getResidentSize();
                    if (isPreloading()) {
                        std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                        layer->_preloadingCache.put(_tile.getTileId(), vtTile, tileSize);
                        if (tileData->getMaxAge() >= 0) {
                            layer->_preloadingCache.invalidate(_tile.getTileId(), std::chrono::steady_clock::now() + std::chrono::milliseconds(tileData->getMaxAge()));
                        }
                    } else {
                        std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                        layer->_visibleCache.put(_tile.getTileId(), vtTile, tileSize);
                        if (tileData->getMaxAge() >= 0) {
                            layer->_visibleCache.invalidate(_tile.getTileId(), std::chrono::steady_clock::now() + std::chrono::milliseconds(tileData->getMaxAge()));
                        }
                    }
                }
                refresh = true; // NOTE: need to refresh even when invalidated
            } else {
                Log::Error("RasterTileLayer::FetchTask: Failed to decode tile");
            }
            break;
        }
        
        return refresh;
    }
    
    std::shared_ptr<Bitmap> RasterTileLayer::FetchTask::extractSubTile(const MapTile& subTile, const MapTile& tile, const std::shared_ptr<Bitmap>& bitmap) {
        int deltaZoom = subTile.getZoom() - tile.getZoom();
        int x = (bitmap->getWidth()  * (subTile.getX() & ((1 << deltaZoom) - 1))) >> deltaZoom;
        int y = (bitmap->getHeight() * (subTile.getY() & ((1 << deltaZoom) - 1))) >> deltaZoom;
        int w = bitmap->getWidth()  >> deltaZoom;
        int h = bitmap->getHeight() >> deltaZoom;
        std::shared_ptr<Bitmap> subBitmap = bitmap->getSubBitmap(x, y, std::max(w, 1), std::max(h, 1));
        return subBitmap->getResizedBitmap(bitmap->getWidth(), bitmap->getHeight());
    }

    std::shared_ptr<vt::Tile> RasterTileLayer::FetchTask::createVectorTile(const MapTile& tile, const std::shared_ptr<Bitmap>& bitmap) {
        std::shared_ptr<vt::TileBitmap> tileBitmap;
        switch (bitmap->getColorFormat()) {
        case ColorFormat::COLOR_FORMAT_GRAYSCALE:
            tileBitmap = std::make_shared<vt::TileBitmap>(vt::TileBitmap::Format::GRAYSCALE, bitmap->getWidth(), bitmap->getHeight(), bitmap->getPixelData());
            break;
        case ColorFormat::COLOR_FORMAT_RGB:
            tileBitmap = std::make_shared<vt::TileBitmap>(vt::TileBitmap::Format::RGB, bitmap->getWidth(), bitmap->getHeight(), bitmap->getPixelData());
            break;
        case ColorFormat::COLOR_FORMAT_RGBA:
            tileBitmap = std::make_shared<vt::TileBitmap>(vt::TileBitmap::Format::RGBA, bitmap->getWidth(), bitmap->getHeight(), bitmap->getPixelData());
            break;
        default:
            tileBitmap = std::make_shared<vt::TileBitmap>(vt::TileBitmap::Format::RGBA, bitmap->getWidth(), bitmap->getHeight(), bitmap->getRGBABitmap()->getPixelData());
            break;
        }

        vt::TileLayerBuilder tileLayerBuilder(256.0f); // Note: the size argument is ignored
        tileLayerBuilder.addBitmap(tileBitmap);
        std::shared_ptr<vt::TileLayer> tileLayer = tileLayerBuilder.build(0, 1.0f, boost::optional<vt::CompOp>());

        vt::TileId vtTile(tile.getZoom(), tile.getX(), tile.getY());
        return std::make_shared<vt::Tile>(vtTile, std::vector<std::shared_ptr<vt::TileLayer> > { tileLayer });
    }

}
