#include "VectorTileLayer.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "components/CancelableThreadPool.h"
#include "datasources/TileDataSource.h"
#include "layers/TileLoadListener.h"
#include "layers/VectorTileEventListener.h"
#include "renderers/MapRenderer.h"
#include "renderers/TileRenderer.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/drawdatas/TileDrawData.h"
#include "utils/Log.h"
#include "utils/Const.h"
#include "ui/VectorTileClickInfo.h"
#include "vectortiles/VectorTileDecoder.h"

#include <vt/TileId.h>
#include <vt/Tile.h>

namespace carto {

    VectorTileLayer::VectorTileLayer(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<VectorTileDecoder>& decoder) :
        TileLayer(dataSource),
        _useFBO(false),
        _useDepth(true),
        _useStencil(true),
        _useTileMapMode(false),
        _vectorTileEventListener(),
        _labelRenderOrder(VectorTileRenderOrder::VECTOR_TILE_RENDER_ORDER_LAYER),
        _buildingRenderOrder(VectorTileRenderOrder::VECTOR_TILE_RENDER_ORDER_LAST),
        _tileDecoder(decoder),
        _tileDecoderListener(),
        _labelCullThreadPool(std::make_shared<CancelableThreadPool>()),
        _renderer(),
        _tempDrawDatas(),
        _visibleCache(128 * 1024 * 1024), // NOTE: the limit should never be reached in normal cases
        _preloadingCache(DEFAULT_PRELOADING_CACHE_SIZE)
    {
        if (!decoder) {
            throw NullArgumentException("Null decoder");
        }

        _labelCullThreadPool->setPoolSize(1);

        setCullDelay(DEFAULT_CULL_DELAY);
    }
    
    VectorTileLayer::~VectorTileLayer() {
        _labelCullThreadPool->cancelAll();
        _labelCullThreadPool->deinit();
    }
    
    std::shared_ptr<VectorTileDecoder> VectorTileLayer::getTileDecoder() const {
        return _tileDecoder;
    }
    
    std::size_t VectorTileLayer::getTileCacheCapacity() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _preloadingCache.capacity();
    }
    
    void VectorTileLayer::setTileCacheCapacity(std::size_t capacityInBytes) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _preloadingCache.resize(capacityInBytes);
    }
    
    VectorTileRenderOrder::VectorTileRenderOrder VectorTileLayer::getLabelRenderOrder() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _labelRenderOrder;
    }
    
    void VectorTileLayer::setLabelRenderOrder(VectorTileRenderOrder::VectorTileRenderOrder renderOrder) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _labelRenderOrder = renderOrder;
        }
        refresh();
    }
    
    VectorTileRenderOrder::VectorTileRenderOrder VectorTileLayer::getBuildingRenderOrder() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _buildingRenderOrder;
    }
    
    void VectorTileLayer::setBuildingRenderOrder(VectorTileRenderOrder::VectorTileRenderOrder renderOrder) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _buildingRenderOrder = renderOrder;
        }
        refresh();
    }
    
    std::shared_ptr<VectorTileEventListener> VectorTileLayer::getVectorTileEventListener() const {
        return _vectorTileEventListener.get();
    }
    
    void VectorTileLayer::setVectorTileEventListener(const std::shared_ptr<VectorTileEventListener>& eventListener) {
        _vectorTileEventListener.set(eventListener);
        tilesChanged(false); // we must reload the tiles, we do not keep full element information if this is not required
    }
    
    bool VectorTileLayer::tileExists(const MapTile& tile, bool preloadingCache) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        long long tileId = getTileId(tile);
        if (preloadingCache) {
            return _preloadingCache.exists(tileId);
        }
        else {
            return _visibleCache.exists(tileId);
        }
    }
    
    bool VectorTileLayer::tileValid(const MapTile& tile, bool preloadingCache) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        long long tileId = getTileId(tile);
        if (preloadingCache) {
            return _preloadingCache.exists(tileId) && _preloadingCache.valid(tileId);
        }
        else {
            return _visibleCache.exists(tileId) && _visibleCache.valid(tileId);
        }
    }
    
    void VectorTileLayer::fetchTile(const MapTile& tile, bool preloadingTile, bool invalidated) {
        long long tileId = getTileId(tile);
        if (_fetchingTiles.exists(tileId)) {
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
        
        auto task = std::make_shared<FetchTask>(std::static_pointer_cast<VectorTileLayer>(shared_from_this()), MapTile(tile.getX(), tile.getY(), tile.getZoom(), 0), preloadingTile);
        _fetchingTiles.add(tileId, task);
        
        std::shared_ptr<CancelableThreadPool> tileThreadPool;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            tileThreadPool = _tileThreadPool;
        }
        if (tileThreadPool) {
            tileThreadPool->execute(task, preloadingTile ? getUpdatePriority() + PRELOADING_PRIORITY_OFFSET : getUpdatePriority());
        }
    }

    void VectorTileLayer::clearTiles(bool preloadingTiles) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (preloadingTiles) {
            _preloadingCache.clear();
        }
        else {
            _visibleCache.clear();
        }
    }

    void VectorTileLayer::tilesChanged(bool removeTiles) {
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
            _preloadingCache.clear();
        }
        refresh();
    }

    long long VectorTileLayer::getTileId(const MapTile& mapTile) const {
        if (_useTileMapMode) {
            return MapTile(mapTile.getX(), mapTile.getY(), mapTile.getZoom(), 0).getTileId();
        } else {
            return mapTile.getTileId();
        }
    }
    
    void VectorTileLayer::calculateDrawData(const MapTile& visTile, const MapTile& closestTile, bool preloadingTile) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        long long closestTileId = getTileId(closestTile);
        TileInfo tileInfo;
        _visibleCache.read(closestTileId, tileInfo);
        if (!tileInfo.getTileMap()) {
            _preloadingCache.read(closestTileId, tileInfo);
        }
        if (std::shared_ptr<VectorTileDecoder::TileMap> tileMap = tileInfo.getTileMap()) {
            auto it = tileMap->find(_useTileMapMode ? closestTile.getFrameNr() : 0);
            if (it != tileMap->end()) {
                std::shared_ptr<const vt::Tile> vtTile = it->second;
                vt::TileId vtTileId(visTile.getZoom(), visTile.getX(), visTile.getY());
                if (closestTile.getZoom() > visTile.getZoom()) {
                    int dx = visTile.getX() >> visTile.getZoom();
                    int dy = visTile.getY() >> visTile.getZoom();
                    vtTileId = vt::TileId(closestTile.getZoom(), closestTile.getX() + (dx << closestTile.getZoom()), closestTile.getY() + (dy << closestTile.getZoom()));
                }
                _tempDrawDatas.push_back(std::make_shared<TileDrawData>(vtTileId, vtTile, closestTileId, preloadingTile));
            }
        }
    }
    
    void VectorTileLayer::refreshDrawData(const std::shared_ptr<CullState>& cullState) {
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
        bool cull = false;
        if (_renderer) {
            if (!(_synchronizedRefresh && _fetchingTiles.getVisibleCount() > 0)) {
                if (_renderer->refreshTiles(_tempDrawDatas)) {
                    refresh = true;
                    cull = true;
                }
            }
        }
    
        if (!_lastCullState || cullState->getViewState().getModelviewProjectionMat() != _lastCullState->getViewState().getModelviewProjectionMat()) {
            cull = true;
        }
    
        if (cull) {
            _labelCullThreadPool->cancelAll();
            std::shared_ptr<CancelableTask> task = std::make_shared<LabelCullTask>(std::static_pointer_cast<VectorTileLayer>(shared_from_this()), _renderer, cullState->getViewState());
            _labelCullThreadPool->execute(task);
        }
    
        if (refresh) {
            if (std::shared_ptr<MapRenderer> mapRenderer = _mapRenderer.lock()) {
                mapRenderer->requestRedraw();
            }
        }
        
        _tempDrawDatas.clear();
    }
    
    int VectorTileLayer::getMinZoom() const {
        return std::max(_dataSource->getMinZoom(), _tileDecoder->getMinZoom());
    }
        
    int VectorTileLayer::getMaxZoom() const {
        return _tileDecoder->getMaxZoom(); // NOTE: datasource max zoom is handled differently
    }    
    
    void VectorTileLayer::calculateRayIntersectedElements(const Projection& projection, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        DirectorPtr<VectorTileEventListener> eventListener = _vectorTileEventListener;

        if (eventListener) {

            for (int pass = 0; pass < 2; pass++) {
                std::vector<std::tuple<vt::TileId, double, long long> > hitResults;
                if (pass == 0) {
                    _renderer->calculateRayIntersectedElements(ray, viewState, hitResults);
                } else {
                    _renderer->calculateRayIntersectedElements3D(ray, viewState, hitResults);
                }

                for (auto it = hitResults.rbegin(); it != hitResults.rend(); it++) {
                    vt::TileId vtTileId = std::get<0>(*it);
                    double t = std::get<1>(*it);
                    long long id = std::get<2>(*it);

                    std::lock_guard<std::recursive_mutex> lock(_mutex);

                    MapTile mapTile(vtTileId.x, vtTileId.y, vtTileId.zoom, _frameNr);
                    MapPos mapPos = projection.fromInternal(MapPos(ray(t)(0), ray(t)(1), ray(t)(2)));

                    TileInfo tileInfo;
                    _visibleCache.peek(getTileId(mapTile), tileInfo);

                    if (std::shared_ptr<BinaryData> tileData = tileInfo.getTileData()) {
                        std::shared_ptr<VectorTileDecoder::TileFeature> tileFeature = _tileDecoder->decodeFeature(id, vtTileId, tileData, tileInfo.getTileBounds());
                        if (tileFeature) {
                            auto elementInfo = std::make_shared<std::pair<MapTile, VectorTileDecoder::TileFeature> >(mapTile, *tileFeature);
                            std::shared_ptr<Layer> thisLayer = std::const_pointer_cast<Layer>(shared_from_this());
                            results.push_back(RayIntersectedElement(elementInfo, thisLayer, mapPos, mapPos, 0, pass > 0));
                        } else {
                            Log::Warnf("VectorTileLayer::calculateRayIntersectedElements: Failed to decode feature %lld", id);
                        }
                    } else {
                        Log::Warn("VectorTileLayer::calculateRayIntersectedElements: Failed to find tile data");
                    }
                }
            }
        }

        TileLayer::calculateRayIntersectedElements(projection, ray, viewState, results);
    }

    bool VectorTileLayer::processClick(ClickType::ClickType clickType, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const {
        DirectorPtr<VectorTileEventListener> eventListener = _vectorTileEventListener;

        if (eventListener) {
            if (std::shared_ptr<std::pair<MapTile, VectorTileDecoder::TileFeature> > elementInfo = intersectedElement.getElement<std::pair<MapTile, VectorTileDecoder::TileFeature> >()) {
                const MapTile& mapTile = elementInfo->first;
                const VectorTileDecoder::TileFeature& tileFeature = elementInfo->second;
                auto clickInfo = std::make_shared<VectorTileClickInfo>(clickType, intersectedElement.getHitPos(), intersectedElement.getHitPos(), mapTile, std::get<0>(tileFeature), std::get<2>(tileFeature), std::get<1>(tileFeature), intersectedElement.getLayer());
                return eventListener->onVectorTileClicked(clickInfo);
            }
        }

        return TileLayer::processClick(clickType, intersectedElement, viewState);
    }

    void VectorTileLayer::offsetLayerHorizontally(double offset) {
        _renderer->offsetLayerHorizontally(offset);
    }
    
    void VectorTileLayer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
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
        _renderer = std::make_shared<TileRenderer>(_mapRenderer, _useFBO, _useDepth, _useStencil);
        _renderer->onSurfaceCreated(shaderManager, textureManager);
        _renderer->setBackgroundColor(_tileDecoder->getBackgroundColor());
        if (_tileDecoder->getBackgroundPattern()) {
            _renderer->setBackgroundPattern(_tileDecoder->getBackgroundPattern());
        }
    }
    
    bool VectorTileLayer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState) {
        updateTileLoadListener();

        _renderer->setLabelOrder(static_cast<int>(getLabelRenderOrder()));
        _renderer->setBuildingOrder(static_cast<int>(getBuildingRenderOrder()));
        _renderer->setInteractionMode(_vectorTileEventListener.get() ? true : false);
        return _renderer->onDrawFrame(deltaSeconds, viewState);
    }
        
    bool VectorTileLayer::onDrawFrame3D(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState) {
        return _renderer->onDrawFrame3D(deltaSeconds, viewState);
    }
    
    void VectorTileLayer::onSurfaceDestroyed() {
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
    
    void VectorTileLayer::registerDataSourceListener() {
        _tileDecoderListener = std::make_shared<TileDecoderListener>(std::static_pointer_cast<VectorTileLayer>(shared_from_this()));
        _tileDecoder->registerOnChangeListener(_tileDecoderListener);
    
        _dataSourceListener = std::make_shared<DataSourceListener>(std::static_pointer_cast<VectorTileLayer>(shared_from_this()));
        _dataSource->registerOnChangeListener(_dataSourceListener);
    }
    
    void VectorTileLayer::unregisterDataSourceListener() {
        _dataSource->unregisterOnChangeListener(_dataSourceListener);
        _dataSourceListener.reset();

        _tileDecoder->unregisterOnChangeListener(_tileDecoderListener);
        _tileDecoderListener.reset();
    }
    
    VectorTileLayer::TileDecoderListener::TileDecoderListener(const std::shared_ptr<VectorTileLayer>& layer) :
        _layer(layer)
    {
    }
        
    void VectorTileLayer::TileDecoderListener::onDecoderChanged() {
        if (std::shared_ptr<VectorTileLayer> layer = _layer.lock()) {
            layer->tilesChanged(false);
        } else {
            Log::Error("VectorTileLayer::TileDecoderListener: Lost connection to layer");
        }
    }
    
    VectorTileLayer::FetchTask::FetchTask(const std::shared_ptr<VectorTileLayer>& layer, const MapTile& tile, bool preloadingTile) :
        FetchTaskBase(layer, tile, preloadingTile)
    {
    }
    
    bool VectorTileLayer::FetchTask::loadTile(const std::shared_ptr<TileLayer>& tileLayer) {
        auto layer = std::static_pointer_cast<VectorTileLayer>(tileLayer);
        
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
    
            vt::TileId vtTile(_tile.getZoom(), _tile.getX(), _tile.getY());
            vt::TileId vtDataSourceTile(dataSourceTile.getZoom(), dataSourceTile.getX(), dataSourceTile.getY());
            std::shared_ptr<VectorTileDecoder::TileMap> tileMap = layer->_tileDecoder->decodeTile(vtDataSourceTile, vtTile, tileData->getData());
            if (tileMap) {
                // Construct tile info - keep original data if interactivity is required
                VectorTileLayer::TileInfo tileInfo(layer->calculateMapTileBounds(dataSourceTile.getFlipped()), layer->_vectorTileEventListener.get() ? tileData->getData() : std::shared_ptr<BinaryData>(), tileMap);

                // Store tile to cache, unless invalidated
                if (!isInvalidated()) {
                    long long tileId = layer->getTileId(_tile);
                    if (isPreloading()) {
                        std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                        layer->_preloadingCache.put(tileId, tileInfo, tileInfo.getSize());
                        if (tileData->getMaxAge() >= 0) {
                            layer->_preloadingCache.invalidate(tileId, std::chrono::steady_clock::now() + std::chrono::milliseconds(tileData->getMaxAge()));
                        }
                    }
                    else {
                        std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                        layer->_visibleCache.put(tileId, tileInfo, tileInfo.getSize());
                        if (tileData->getMaxAge() >= 0) {
                            layer->_visibleCache.invalidate(tileId, std::chrono::steady_clock::now() + std::chrono::milliseconds(tileData->getMaxAge()));
                        }
                    }
                }
                
                // Debug tile performance issues
                if (Log::IsShowDebug()) {
                    int maxDrawCallCount = 0;
                    for (auto it = tileMap->begin(); it != tileMap->end(); it++) {
                        int drawCallCount = 0;
                        for (const std::shared_ptr<vt::TileLayer>& vtLayer : it->second->getLayers()) {
                            drawCallCount += static_cast<int>(vtLayer->getBitmaps().size() + vtLayer->getGeometries().size());
                        }
                        maxDrawCallCount = std::max(maxDrawCallCount, drawCallCount);
                    }
                    if (maxDrawCallCount >= 20) {
                        Log::Debugf("VectorTileLayer::FetchTask: Tile requires %d draw calls", maxDrawCallCount);
                    }
                }
                
                refresh = true; // NOTE: need to refresh even when invalidated
            } else {
                Log::Error("VectorTileLayer::FetchTask: Failed to decode tile");
            }
            break;
        }
        
        return refresh;
    }
        
    VectorTileLayer::LabelCullTask::LabelCullTask(const std::shared_ptr<VectorTileLayer>& layer, const std::shared_ptr<TileRenderer>& renderer, const ViewState& viewState) : _layer(layer), _renderer(renderer), _viewState(viewState)
    {
    }
        
    void VectorTileLayer::LabelCullTask::cancel() {
    }
        
    void VectorTileLayer::LabelCullTask::run() {
        std::shared_ptr<VectorTileLayer> layer = _layer.lock();
        if (!layer) {
            return;
        }
    
        if (std::shared_ptr<TileRenderer> renderer = _renderer.lock()) {
            if (renderer->cullLabels(_viewState)) {
                if (auto mapRenderer = layer->_mapRenderer.lock()) {
                    mapRenderer->requestRedraw();
                }
            }
        }
    }

    std::size_t VectorTileLayer::TileInfo::getSize() const {
        std::size_t size = EXTRA_TILE_FOOTPRINT;
        if (_tileData) {
            size += _tileData->size();
        }
        for (auto it = _tileMap->begin(); it != _tileMap->end(); it++) {
            size += it->second->getResidentSize();
        }
        return size;
    }
    
}
