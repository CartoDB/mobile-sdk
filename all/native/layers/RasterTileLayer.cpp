#include "RasterTileLayer.h"
#include "components/Exceptions.h"
#include "components/CancelableThreadPool.h"
#include "datasources/TileDataSource.h"
#include "layers/RasterTileEventListener.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "renderers/MapRenderer.h"
#include "renderers/TileRenderer.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/drawdatas/TileDrawData.h"
#include "graphics/Bitmap.h"
#include "ui/RasterTileClickInfo.h"
#include "utils/Log.h"
#include "utils/Const.h"

#include <array>
#include <algorithm>

#include <vt/TileId.h>
#include <vt/Tile.h>
#include <vt/TileTransformer.h>
#include <vt/TileBitmap.h>
#include <vt/TileLayer.h>
#include <vt/TileLayerBuilder.h>

namespace {

    std::array<std::uint8_t, 4> readTileBitmapColor(const carto::vt::TileBitmap& bitmap, int x, int y) {
        x = std::max(0, std::min(x, bitmap.getWidth() - 1));
        y = bitmap.getHeight() - 1 - std::max(0, std::min(y, bitmap.getHeight() - 1));

        switch (bitmap.getFormat()) {
        case carto::vt::TileBitmap::Format::GRAYSCALE:
            {
                std::uint8_t val = bitmap.getData()[y * bitmap.getWidth() + x];
                return std::array<std::uint8_t, 4> { { val, val, val, 255 } };
            }
        case carto::vt::TileBitmap::Format::RGB:
            {
                const std::uint8_t* valPtr = &bitmap.getData()[(y * bitmap.getWidth() + x) * 3];
                return std::array<std::uint8_t, 4> { { valPtr[0], valPtr[1], valPtr[2], 255 } };
            }
        case carto::vt::TileBitmap::Format::RGBA:
            {
                const std::uint8_t* valPtr = &bitmap.getData()[(y * bitmap.getWidth() + x) * 4];
                return std::array<std::uint8_t, 4> { { valPtr[0], valPtr[1], valPtr[2], valPtr[3] } };
            }
        default:
            break;
        }
        return std::array<std::uint8_t, 4> { { 0, 0, 0, 0 } };
    }

    std::array<std::uint8_t, 4> readTileBitmapColor(const carto::vt::TileBitmap& bitmap, float x, float y) {
        std::array<float, 4> result { { 0, 0, 0, 0 } };
        for (int dy = 0; dy < 2; dy++) {
            for (int dx = 0; dx < 2; dx++) {
                int x0 = static_cast<int>(std::floor(x));
                int y0 = static_cast<int>(std::floor(y));

                std::array<std::uint8_t, 4> color = readTileBitmapColor(bitmap, x0 + dx, y0 + dy);
                for (int i = 0; i < 4; i++) {
                    result[i] += color[i] * (dx == 0 ? x0 + 1.0f - x : x - x0) * (dy == 0 ? y0 + 1.0f - y : y - y0);
                }
            }
        }
        return std::array<std::uint8_t, 4> { { static_cast<std::uint8_t>(result[0]), static_cast<std::uint8_t>(result[1]), static_cast<std::uint8_t>(result[2]), static_cast<std::uint8_t>(result[3]) } };
    }

}

namespace carto {

    RasterTileLayer::RasterTileLayer(const std::shared_ptr<TileDataSource>& dataSource) :
        TileLayer(dataSource),
        _tileFilterMode(RasterTileFilterMode::RASTER_TILE_FILTER_MODE_BILINEAR),
        _rasterTileEventListener(),
        _visibleTileIds(),
        _tempDrawDatas(),
        _visibleCache(128 * 1024 * 1024), // limit should be never reached during normal use cases
        _preloadingCache(DEFAULT_PRELOADING_CACHE_SIZE)
    {
        setCullDelay(DEFAULT_CULL_DELAY);
    }
    
    RasterTileLayer::~RasterTileLayer() {
    }
    
    std::size_t RasterTileLayer::getTextureCacheCapacity() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _preloadingCache.capacity();
    }
    
    void RasterTileLayer::setTextureCacheCapacity(std::size_t capacityInBytes) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _preloadingCache.resize(capacityInBytes);
    }
    
    RasterTileFilterMode::RasterTileFilterMode RasterTileLayer::getTileFilterMode() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _tileFilterMode;
    }

    void RasterTileLayer::setTileFilterMode(RasterTileFilterMode::RasterTileFilterMode filterMode) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _tileFilterMode = filterMode;
        }
        redraw();
    }

    std::shared_ptr<RasterTileEventListener> RasterTileLayer::getRasterTileEventListener() const {
        return _rasterTileEventListener.get();
    }
    
    void RasterTileLayer::setRasterTileEventListener(const std::shared_ptr<RasterTileEventListener>& eventListener) {
        std::shared_ptr<RasterTileEventListener> oldEventListener = _rasterTileEventListener.get();
        _rasterTileEventListener.set(eventListener);
        _tileRenderer->setInteractionMode(eventListener.get() ? true : false);
        if (eventListener && !oldEventListener) {
            tilesChanged(false); // we must reload the tiles, we do not keep full element information if this is not required
        }
    }
    
    bool RasterTileLayer::tileExists(const MapTile& tile, bool preloadingCache) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        long long tileId = tile.getTileId();
        if (preloadingCache) {
            return _preloadingCache.exists(tileId);
        } else {
            return _visibleCache.exists(tileId);
        }
    }
    
    bool RasterTileLayer::tileValid(const MapTile& tile, bool preloadingCache) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        long long tileId = tile.getTileId();
        if (preloadingCache) {
            return _preloadingCache.exists(tileId) && _preloadingCache.valid(tileId);
        } else {
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
            if (_preloadingCache.exists(tileId) && _preloadingCache.valid(tileId)) {
                if (!preloadingTile) {
                    _preloadingCache.move(tileId, _visibleCache); // move to visible cache, just in case the element gets trashed
                } else {
                    _preloadingCache.get(tileId);
                }
                return;
            }
    
            if (_visibleCache.exists(tileId) && _visibleCache.valid(tileId)) {
                _visibleCache.get(tileId); // just mark usage, do not move to preloading, it will be moved at later stage
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
        } else {
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
        } else {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _visibleCache.invalidate_all(std::chrono::steady_clock::now());
            _preloadingCache.clear();
        }
        refresh();
    }

    vt::RasterFilterMode RasterTileLayer::getRasterFilterMode() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        switch (_tileFilterMode) {
        case RasterTileFilterMode::RASTER_TILE_FILTER_MODE_NEAREST:
            return vt::RasterFilterMode::NEAREST;
        case RasterTileFilterMode::RASTER_TILE_FILTER_MODE_BICUBIC:
            return vt::RasterFilterMode::BICUBIC;
        default:
            return vt::RasterFilterMode::BILINEAR;
        }
    }

    std::shared_ptr<vt::Tile> RasterTileLayer::createVectorTile(const MapTile& tile, const std::shared_ptr<Bitmap>& bitmap) const {
        // Build tile bitmap from the original bitmap, by doing conversion, if necessary
        std::shared_ptr<vt::TileBitmap> tileBitmap;
        switch (bitmap->getColorFormat()) {
        case ColorFormat::COLOR_FORMAT_GRAYSCALE:
            tileBitmap = std::make_shared<vt::TileBitmap>(vt::TileBitmap::Type::COLORMAP, vt::TileBitmap::Format::GRAYSCALE, bitmap->getWidth(), bitmap->getHeight(), bitmap->getPixelData());
            break;
        case ColorFormat::COLOR_FORMAT_RGB:
            tileBitmap = std::make_shared<vt::TileBitmap>(vt::TileBitmap::Type::COLORMAP, vt::TileBitmap::Format::RGB, bitmap->getWidth(), bitmap->getHeight(), bitmap->getPixelData());
            break;
        case ColorFormat::COLOR_FORMAT_RGBA:
            tileBitmap = std::make_shared<vt::TileBitmap>(vt::TileBitmap::Type::COLORMAP, vt::TileBitmap::Format::RGBA, bitmap->getWidth(), bitmap->getHeight(), bitmap->getPixelData());
            break;
        default:
            tileBitmap = std::make_shared<vt::TileBitmap>(vt::TileBitmap::Type::COLORMAP, vt::TileBitmap::Format::RGBA, bitmap->getWidth(), bitmap->getHeight(), bitmap->getRGBABitmap()->getPixelData());
            break;
        }

        // Build actual vector tile using created colormap
        float tileSize = 256.0f; // 'normalized' tile size in pixels. Not really important
        vt::TileId vtTile(tile.getZoom(), tile.getX(), tile.getY());
        std::shared_ptr<vt::TileBackground> tileBackground = std::make_shared<vt::TileBackground>(vt::Color(), std::shared_ptr<vt::BitmapPattern>());
        std::shared_ptr<const vt::TileTransformer::VertexTransformer> vtTransformer = getTileTransformer()->createTileVertexTransformer(vtTile);
        vt::TileLayerBuilder tileLayerBuilder(vtTile, 0, vtTransformer, tileSize, 1.0f); // Note: the size/scale argument is ignored
        tileLayerBuilder.addBitmap(tileBitmap);
        std::shared_ptr<vt::TileLayer> tileLayer = tileLayerBuilder.buildTileLayer(boost::optional<vt::CompOp>(), vt::FloatFunction(1));
        return std::make_shared<vt::Tile>(vtTile, tileSize, tileBackground, std::vector<std::shared_ptr<vt::TileLayer> > { tileLayer });
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
        if (!(_synchronizedRefresh && _fetchingTiles.getVisibleCount() > 0)) {
            if (_tileRenderer->refreshTiles(_tempDrawDatas)) {
                refresh = true;
            }
        }
    
        if (refresh) {
            redraw();
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _visibleTileIds.clear();
            for (const std::shared_ptr<TileDrawData>& drawData : _tempDrawDatas) {
                _visibleTileIds.push_back(drawData->getTileId());
            }
            _tempDrawDatas.clear();
        }
    }
    
    int RasterTileLayer::getMinZoom() const {
        return _dataSource->getMinZoom();
    }
    
    int RasterTileLayer::getMaxZoom() const {
        return _dataSource->getMaxZoom();
    }

    std::vector<long long> RasterTileLayer::getVisibleTileIds() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _visibleTileIds;
    }
        
    void RasterTileLayer::calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        DirectorPtr<RasterTileEventListener> eventListener = _rasterTileEventListener;

        if (eventListener) {
            std::vector<vt::GLTileRenderer::BitmapIntersectionInfo> hitResults;
            _tileRenderer->calculateRayIntersectedBitmaps(ray, viewState, hitResults);

            for (const vt::GLTileRenderer::BitmapIntersectionInfo& hitResult : hitResults) {
                std::shared_ptr<const vt::TileBitmap> tileBitmap = hitResult.bitmap;
                if (!tileBitmap || tileBitmap->getData().empty() || tileBitmap->getWidth() < 1 || tileBitmap->getHeight() < 1) {
                    Log::Warnf("RasterTileLayer::calculateRayIntersectedElements: Bitmap data not available");
                    continue;
                }

                std::lock_guard<std::recursive_mutex> lock(_mutex);

                float x = hitResult.uv(0) * tileBitmap->getWidth();
                float y = hitResult.uv(1) * tileBitmap->getHeight();
                std::array<std::uint8_t, 4> interpolatedComponents = readTileBitmapColor(*tileBitmap, x - 0.5f, y - 0.5f);
                Color interpolatedColor(interpolatedComponents[0], interpolatedComponents[1], interpolatedComponents[2], interpolatedComponents[3]);

                int nx = static_cast<int>(std::floor(x));
                int ny = static_cast<int>(std::floor(y));
                std::array<std::uint8_t, 4> nearestComponents = readTileBitmapColor(*tileBitmap, nx, ny);
                Color nearestColor(nearestComponents[0], nearestComponents[1], nearestComponents[2], nearestComponents[3]);

                auto pixelInfo = std::make_shared<std::tuple<MapTile, Color, Color> >(MapTile(hitResult.tileId.x, hitResult.tileId.y, hitResult.tileId.zoom, _frameNr), nearestColor, interpolatedColor);
                std::shared_ptr<Layer> thisLayer = std::const_pointer_cast<Layer>(shared_from_this());
                results.push_back(RayIntersectedElement(pixelInfo, thisLayer, ray(hitResult.rayT), ray(hitResult.rayT), false));
            }
        }

        TileLayer::calculateRayIntersectedElements(ray, viewState, results);
    }

    bool RasterTileLayer::processClick(ClickType::ClickType clickType, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const {
        std::shared_ptr<ProjectionSurface> projectionSurface = viewState.getProjectionSurface();
        if (!projectionSurface) {
            return false;
        }
        
        DirectorPtr<RasterTileEventListener> eventListener = _rasterTileEventListener;

        if (eventListener) {
            if (auto pixelInfo = intersectedElement.getElement<std::tuple<MapTile, Color, Color> >()) {
                const MapTile& mapTile = std::get<0>(*pixelInfo);
                const Color& nearestColor = std::get<1>(*pixelInfo);
                const Color& interpolatedColor = std::get<2>(*pixelInfo);
                MapPos hitPos = _dataSource->getProjection()->fromInternal(projectionSurface->calculateMapPos(intersectedElement.getHitPos()));

                auto clickInfo = std::make_shared<RasterTileClickInfo>(clickType, hitPos, mapTile, nearestColor, interpolatedColor, intersectedElement.getLayer());
                return eventListener->onRasterTileClicked(clickInfo);
            }
        }

        return TileLayer::processClick(clickType, intersectedElement, viewState);
    }

    void RasterTileLayer::offsetLayerHorizontally(double offset) {
        _tileRenderer->offsetLayerHorizontally(offset);
    }
    
    bool RasterTileLayer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState) {
        updateTileLoadListener();

        if (auto mapRenderer = getMapRenderer()) {
            float opacity = getOpacity();

            if (opacity < 1.0f) {
                mapRenderer->clearAndBindScreenFBO(Color(0, 0, 0, 0), false, false);
            }

            _tileRenderer->setRasterFilterMode(getRasterFilterMode());
            bool refresh = _tileRenderer->onDrawFrame(deltaSeconds, viewState);

            if (opacity < 1.0f) {
                mapRenderer->blendAndUnbindScreenFBO(opacity);
            }

            return refresh;
        }
        return false;
    }
    
    bool RasterTileLayer::onDrawFrame3D(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState) {
        return _tileRenderer->onDrawFrame3D(deltaSeconds, viewState);
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
            if (!tileData->getData()) {
                break;
            }
    
            // Save tile to texture cache, unless invalidated
            vt::TileId vtTile(_tile.getZoom(), _tile.getX(), _tile.getY());
            vt::TileId vtDataSourceTile(dataSourceTile.getZoom(), dataSourceTile.getX(), dataSourceTile.getY());
            std::shared_ptr<Bitmap> bitmap = Bitmap::CreateFromCompressed(tileData->getData());
            if (bitmap) {
                // Check if we received the requested tile or extract/scale the corresponding part
                if (dataSourceTile != _tile) {
                    bitmap = ExtractSubTile(_tile, dataSourceTile, bitmap);
                }
                std::shared_ptr<vt::TileTransformer> tileTransformer = layer->getTileTransformer();
                std::shared_ptr<vt::Tile> vtTile = layer->createVectorTile(_tile, bitmap);
                std::size_t vtTileSize = EXTRA_TILE_FOOTPRINT + vtTile->getResidentSize();

                if (!isInvalidated()) {
                    // Build the bitmap object
                    std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                    if (layer->getTileTransformer() == tileTransformer) { // extra check that the tile is created with correct transformer. Otherwise simply drop it.
                        if (isPreloading()) {
                            layer->_preloadingCache.put(_tile.getTileId(), vtTile, vtTileSize);
                            if (tileData->getMaxAge() >= 0) {
                                layer->_preloadingCache.invalidate(_tile.getTileId(), std::chrono::steady_clock::now() + std::chrono::milliseconds(tileData->getMaxAge()));
                            }
                        } else {
                            layer->_visibleCache.put(_tile.getTileId(), vtTile, vtTileSize);
                            if (tileData->getMaxAge() >= 0) {
                                layer->_visibleCache.invalidate(_tile.getTileId(), std::chrono::steady_clock::now() + std::chrono::milliseconds(tileData->getMaxAge()));
                            }
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
    
    std::shared_ptr<Bitmap> RasterTileLayer::FetchTask::ExtractSubTile(const MapTile& subTile, const MapTile& tile, const std::shared_ptr<Bitmap>& bitmap) {
        int deltaZoom = subTile.getZoom() - tile.getZoom();
        int x = (bitmap->getWidth()  * (subTile.getX() & ((1 << deltaZoom) - 1))) >> deltaZoom;
        int y = (bitmap->getHeight() * (subTile.getY() & ((1 << deltaZoom) - 1))) >> deltaZoom;
        int w = bitmap->getWidth()  >> deltaZoom;
        int h = bitmap->getHeight() >> deltaZoom;
        std::shared_ptr<Bitmap> subBitmap = bitmap->getSubBitmap(x, y, std::max(w, 1), std::max(h, 1));
        return subBitmap->getResizedBitmap(bitmap->getWidth(), bitmap->getHeight());
    }

    const int RasterTileLayer::DEFAULT_CULL_DELAY = 200;
    const int RasterTileLayer::PRELOADING_PRIORITY_OFFSET = -2;

    const unsigned int RasterTileLayer::EXTRA_TILE_FOOTPRINT = 4096;
    const unsigned int RasterTileLayer::DEFAULT_PRELOADING_CACHE_SIZE = 10 * 1024 * 1024;

}
