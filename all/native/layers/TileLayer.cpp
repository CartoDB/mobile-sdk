#include "TileLayer.h"
#include "components/CancelableTask.h"
#include "layers/TileLoadListener.h"
#include "renderers/components/CullState.h"
#include "renderers/MapRenderer.h"
#include "projections/Projection.h"
#include "utils/Const.h"
#include "utils/GeomUtils.h"
#include "utils/Log.h"

#include <sstream>

namespace carto {

    TileLayer::~TileLayer() {
    }
    
    std::shared_ptr<TileDataSource> TileLayer::getDataSource() const {
        return _dataSource.get();
    }
    
    int TileLayer::getFrameNr() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _frameNr;
    }
    
    void TileLayer::setFrameNr(int frameNr) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _lastFrameNr = _frameNr;
            _frameNr = frameNr;
        }
        refresh();
    }
    
    bool TileLayer::isPreloading() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _preloading;
    }
    
    void TileLayer::setPreloading(bool preloading) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _preloading = preloading;
        }
        refresh();
    }
    
    bool TileLayer::isSynchronizedRefresh() const {
        return _synchronizedRefresh;
    }
    
    void TileLayer::setSynchronizedRefresh(bool synchronizedRefresh) {
        _synchronizedRefresh = synchronizedRefresh;
    }
    
    TileSubstitutionPolicy::TileSubstitutionPolicy TileLayer::getTileSubstitutionPolicy() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _substitutionPolicy;
    }
    
    void TileLayer::setTileSubstitutionPolicy(TileSubstitutionPolicy::TileSubstitutionPolicy policy) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _substitutionPolicy = policy;
        }
        refresh();
    }
        
    float TileLayer::getZoomLevelBias() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _zoomLevelBias;
    }
    
    void TileLayer::setZoomLevelBias(float bias) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _zoomLevelBias = bias;
        }
        refresh();
    }
    
    MapTile TileLayer::calculateMapTile(const MapPos& mapPos, int zoom) const {
        double tileWidth = _dataSource->getProjection()->getBounds().getDelta().getX() / (1 << zoom);
        double tileHeight = _dataSource->getProjection()->getBounds().getDelta().getY() / (1 << zoom);
        MapVec mapVec = mapPos - _dataSource->getProjection()->getBounds().getMin();
        int x = static_cast<int>(std::floor(mapVec.getX() / tileWidth));
        int y = static_cast<int>(std::floor(mapVec.getY() / tileHeight));
        return MapTile(x, y, zoom, 0);
    }
    
    MapPos TileLayer::calculateMapTileOrigin(const MapTile& mapTile) const {
        double tileWidth = _dataSource->getProjection()->getBounds().getDelta().getX() / (1 << mapTile.getZoom());
        double tileHeight = _dataSource->getProjection()->getBounds().getDelta().getY() / (1 << mapTile.getZoom());
        MapVec mapVec(mapTile.getX() * tileWidth, mapTile.getY() * tileHeight);
        return _dataSource->getProjection()->getBounds().getMin() + mapVec;
    }

    MapBounds TileLayer::calculateMapTileBounds(const MapTile& mapTile) const {
        MapPos pos0 = calculateMapTileOrigin(mapTile);
        MapPos pos1 = calculateMapTileOrigin(MapTile(mapTile.getX() + 1, mapTile.getY() + 1, mapTile.getZoom(), mapTile.getFrameNr()));
        return MapBounds(pos0, pos1);
    }
    
    std::shared_ptr<TileLoadListener> TileLayer::getTileLoadListener() const {
        std::lock_guard<std::mutex> lock(_tileLoadListenerMutex);
        return _tileLoadListener.get();
    }
    
    void TileLayer::setTileLoadListener(const std::shared_ptr<TileLoadListener>& tileLoadListener) {
        std::lock_guard<std::mutex> lock(_tileLoadListenerMutex);
        _tileLoadListener = DirectorPtr<TileLoadListener>(tileLoadListener);
    }
    
    bool TileLayer::isUpdateInProgress() const {
        return !_fetchingTiles.getTasks().empty();
    }
    
    TileLayer::DataSourceListener::DataSourceListener(const std::shared_ptr<TileLayer>& layer) :
        _layer(layer)
    {
    }
        
    void TileLayer::DataSourceListener::onTilesChanged(bool removeTiles) {
        if (std::shared_ptr<TileLayer> layer = _layer.lock()) {
            layer->tilesChanged(removeTiles);
        } else {
            Log::Error("TileLayer::DataSourceListener: Lost connection to layer");
        }
    }
        
    TileLayer::TileLayer(const std::shared_ptr<TileDataSource>& dataSource) :
       	Layer(),
        _synchronizedRefresh(false),
        _calculatingTiles(false),
        _refreshedTiles(false),
        _dataSource(dataSource),
        _dataSourceListener(),
        _tileLoadListener(),
        _tileLoadListenerMutex(),
        _fetchingTiles(),
        _frameNr(0),
        _lastFrameNr(-1),
        _preloading(false),
        _substitutionPolicy(TileSubstitutionPolicy::TILE_SUBSTITUTION_POLICY_ALL),
        _zoomLevelBias(0.0f),
        _visibleTiles(),
        _preloadingTiles()
    {
    }
    
    void TileLayer::loadData(const std::shared_ptr<CullState>& cullState) {
        // This method (from update() or refresh()) might be called from multiple threads
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        _calculatingTiles = true;
    
        // Cancel old tasks
        for (const std::shared_ptr<FetchTaskBase>& task : _fetchingTiles.getTasks()) {
            task->cancel();
        }
        
        // Check if layer should be drawn
        if (!_visible || !_visibleZoomRange.inRange(cullState->getViewState().getZoom())) {
            _calculatingTiles = false;

            refreshDrawData(cullState);
            return;
        }
        
        if (!_lastCullState || _frameNr != _lastFrameNr || cullState->getViewState().getModelviewProjectionMat() != _lastCullState->getViewState().getModelviewProjectionMat()) {
            // If the view has changed calculate new visible tiles, otherwise use the old ones
            calculateVisibleTiles(cullState);
        }
    
        // Find replacements for visible tiles
        findTiles(_visibleTiles, false);
    
        if (_preloading) {
            // Find replacements for preloading tiles
            findTiles(_preloadingTiles, true);
            
            // Pre-fetch parent tiles
            std::vector<MapTile> allTiles = _visibleTiles;
            allTiles.insert(allTiles.end(), _preloadingTiles.begin(), _preloadingTiles.end());
            for (const MapTile& visTile : allTiles) {
                if (visTile.getZoom() > 0) {
                    int tileMask = (1 << visTile.getZoom()) - 1;
                    MapTile tile(visTile.getX() & tileMask, visTile.getY() & tileMask, visTile.getZoom(), visTile.getFrameNr());
                    fetchTile(tile.getParent(), true, false);
                }
            }
        }
    
        _calculatingTiles = false;
        _refreshedTiles = true;
        
        refreshDrawData(cullState);
    }

    void TileLayer::updateTileLoadListener() {
        bool calculatingTiles = _calculatingTiles;
    
        DirectorPtr<TileLoadListener> tileLoadListener;
        {
            std::lock_guard<std::mutex> lock(_tileLoadListenerMutex);
            tileLoadListener = _tileLoadListener;
        }
        if (!calculatingTiles && tileLoadListener) {
            bool refreshedTiles = std::atomic_exchange(&_refreshedTiles, false);
    
            // Check if visible tiles have finished loading, notify listener
            if (refreshedTiles && _fetchingTiles.getVisibleCount() == 0) {
                tileLoadListener->onVisibleTilesLoaded();
            }
    
            // Check if preloading tiles have finished loading, notify listener
            if (isPreloading() && refreshedTiles && _fetchingTiles.getPreloadingCount() == 0) {
                tileLoadListener->onPreloadingTilesLoaded();
            }
        }
    }
    
    void TileLayer::calculateVisibleTiles(const std::shared_ptr<CullState>& cullState) {
        // Remove last visible and preloading tiles
        _visibleTiles.clear();
        _preloadingTiles.clear();
        
        // Recursively calculate visible tiles
        calculateVisibleTilesRecursive(cullState, MapTile(0, 0, 0, _frameNr));
        if (auto options = _options.lock()) {
            if (options->isSeamlessPanning()) {
                // Additional visibility testing has to be done if seamless panning is enabled
                calculateVisibleTilesRecursive(cullState, MapTile(-1, 0, 0, _frameNr));
                calculateVisibleTilesRecursive(cullState, MapTile( 1, 0, 0, _frameNr));
            }
        }
        
        // Sort tiles by distance to the camera
        auto distanceComparator = [&](const MapTile& tile1, const MapTile& tile2) {
            MapPos center1 = calculateInternalTileBounds(tile1).getCenter();
            MapPos center2 = calculateInternalTileBounds(tile2).getCenter();
            double dist1 = (center1 - cullState->getViewState().getCameraPos()).length();
            double dist2 = (center2 - cullState->getViewState().getCameraPos()).length();
            return dist1 < dist2;
        };

        std::sort(_visibleTiles.begin(), _visibleTiles.end(), distanceComparator);
        std::sort(_preloadingTiles.begin(), _preloadingTiles.end(), distanceComparator);
    }
    
    void TileLayer::calculateVisibleTilesRecursive(const std::shared_ptr<CullState>& cullState, const MapTile& tile) {
        const ViewState& viewState = cullState->getViewState();
        const Frustum& visibleFrustum = viewState.getFrustum();
        
        if (tile.getZoom() > Const::MAX_SUPPORTED_ZOOM_LEVEL) {
            return;
        }
        
        MapBounds tileBounds = calculateInternalTileBounds(tile);
        MapPos tileCenter = tileBounds.getCenter();

        bool inPreloadingFrustum = visibleFrustum.circleIntersects(tileCenter, tileBounds.getDelta().length() * 0.5 * PRELOADING_TILE_SCALE);
        if (!inPreloadingFrustum) {
            return;
        }
        bool inVisibleFrustum = visibleFrustum.squareIntersects(tileBounds);
        
        // Map tile is visible, calculate distance using camera plane
        const cglib::mat4x4<double>& mvpMat = viewState.getModelviewProjectionMat();
        double tileW = tileCenter.getX() * mvpMat(3, 0) + tileCenter.getY() * mvpMat(3, 1) + mvpMat(3, 3);
        double zoomDistance = tileW * std::pow(2.0f, tile.getZoom() - getZoomLevelBias());
        bool subDivide = zoomDistance < SUBDIVISION_THRESHOLD * Const::SQRT_2;
        int targetTileZoom = std::min(getMaxZoom(), static_cast<int>(viewState.getZoom() + getZoomLevelBias() + DISCRETE_ZOOM_LEVEL_BIAS));
        if (getMinZoom() > tile.getZoom()) {
            subDivide = true;
        } else if (targetTileZoom <= tile.getZoom()) {
            subDivide = false;
        }
        
        if (subDivide) {
            // The tile is too coarse, keep subdividing
            for (int n = 0; n < 4; n++) {
                calculateVisibleTilesRecursive(cullState, tile.getChild(n));
            }
        } else {
            // Add the tile to visible tiles, sort by the distnace to the camera
            if (inVisibleFrustum) {
                _visibleTiles.push_back(tile);
            } else {
                _preloadingTiles.push_back(tile);
            }
        }
    }
    
    void TileLayer::findTiles(const std::vector<MapTile>& visTiles, bool preloadingTiles) {
        for (const MapTile& visTile : visTiles) {
            int tileMask = (1 << visTile.getZoom()) - 1;
            MapTile tile(visTile.getX() & tileMask, visTile.getY() & tileMask, visTile.getZoom(), visTile.getFrameNr());

            // Check caches
            if (tileExists(tile, preloadingTiles) || tileExists(tile, !preloadingTiles)) {
                calculateDrawData(visTile, tile, preloadingTiles);

                // Re-fetch invalid tile
                if (!tileIsValid(tile)) {
                    fetchTile(tile, preloadingTiles, true);
                }
                continue;
            }
            
            // Build list of caches to use (based on tile substitution policy)
            std::vector<bool> preloadingCaches;
            switch (getTileSubstitutionPolicy()) {
                case TileSubstitutionPolicy::TILE_SUBSTITUTION_POLICY_ALL:
                    preloadingCaches.push_back(false);
                    preloadingCaches.push_back(true);
                    break;
                case TileSubstitutionPolicy::TILE_SUBSTITUTION_POLICY_VISIBLE:
                    preloadingCaches.push_back(false);
                    if (preloadingTiles) {
                        preloadingCaches.push_back(true);
                    }
                    break;
                default:
                    break;
            }
            for (bool preloadingCache : preloadingCaches) {
                // Check for a tile with the last frame nr
                MapTile prevFrameTile(tile.getX(), tile.getY(), tile.getZoom(), _lastFrameNr);
                bool foundSubstitute = tileExists(prevFrameTile, preloadingCache);

                if (foundSubstitute) {
                    calculateDrawData(visTile, prevFrameTile, preloadingTiles);
                } else {
                    // Check cache for parent tile
                    if (tile.getZoom() > 0) {
                        foundSubstitute = findParentTile(visTile, tile, MAX_PARENT_SEARCH_DEPTH, preloadingCache, preloadingTiles);
                    }
                    if (!foundSubstitute) {
                        // Didn't find parent tile, check cache for children tiles
                        foundSubstitute = findChildTiles(visTile, tile, MAX_CHILD_SEARCH_DEPTH, preloadingCache, preloadingTiles) > 0;
                    }
                }
                if (foundSubstitute) {
                    break;
                }
            }
    
            // Finally fetch the tile from source
            fetchTile(tile, preloadingTiles, false);
        }
    }
    
    bool TileLayer::findParentTile(const MapTile& visTile, const MapTile& tile, int depth, bool preloadingCache, bool preloadingTile) {
        if (tile.getZoom() <= 0 || depth <= 0) {
            return false;
        }

        MapTile parentTile = tile.getParent();
        
        // Check the cache
        if (tileExists(parentTile, preloadingCache)) {
            calculateDrawData(visTile, parentTile, preloadingTile);
            return true;
        }
    
        // Dind't find the parent in the cache
        return findParentTile(visTile, parentTile, depth - 1, preloadingCache, preloadingTile);
    }
    
    int TileLayer::findChildTiles(const MapTile& visTile, const MapTile& tile, int depth, bool preloadingCache, bool preloadingTile) {
        if (depth <= 0) {
            return 0;
        }
        
        int childTileCount = 0;
        for (int n = 0; n < 4; n++) {
            MapTile subTile = tile.getChild(n);
            if (tileExists(subTile, preloadingCache)) {
                calculateDrawData(visTile, subTile, preloadingTile);
                childTileCount++;
            }
            else {
                childTileCount += findChildTiles(visTile, subTile, depth - 1, preloadingCache, preloadingTile);
            }
        }
        return childTileCount;
    }
    
    MapBounds TileLayer::calculateInternalTileBounds(const MapTile& tile) const {
        MapBounds tileBoundsProj = calculateMapTileBounds(tile);
        MapPos tilePos0 = _dataSource->getProjection()->toInternal(tileBoundsProj.getMin());
        MapPos tilePos1 = _dataSource->getProjection()->toInternal(tileBoundsProj.getMax());
        return MapBounds(MapPos(std::min(tilePos0.getX(), tilePos1.getX()), std::min(-tilePos0.getY(), -tilePos1.getY())), MapPos(std::max(tilePos0.getX(), tilePos1.getX()), std::max(-tilePos0.getY(), -tilePos1.getY())));
    }

    TileLayer::FetchTaskBase::FetchTaskBase(const std::shared_ptr<TileLayer>& layer, const MapTile& tile, bool preloadingTile) :
        _layer(layer),
        _tile(tile),
        _dataSourceTiles(),
        _preloadingTile(preloadingTile),
        _started(false),
        _invalidated(false)
    {
        for (MapTile dataSourceTile = tile; true; ) {
            int zoom = dataSourceTile.getZoom();
            if (zoom >= layer->getDataSource()->getMinZoom() && zoom <= layer->getDataSource()->getMaxZoom()) {
                _dataSourceTiles.push_back(dataSourceTile);
            }
            if (zoom <= 0) {
                break;
            }
            dataSourceTile = dataSourceTile.getParent();
        }
    }
    
    bool TileLayer::FetchTaskBase::isPreloading() const {
        return _preloadingTile;
    }
    
    bool TileLayer::FetchTaskBase::isInvalidated() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _invalidated;
    }

    void TileLayer::FetchTaskBase::invalidate() {
        std::lock_guard<std::mutex> lock(_mutex);
        _invalidated = true;
    }
        
    void TileLayer::FetchTaskBase::cancel() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_started) {
            _canceled = true;
                
            if (std::shared_ptr<TileLayer> layer = _layer.lock()) {
                layer->_fetchingTiles.remove(_tile.getTileId());
            }
        }
    }
        
    void TileLayer::FetchTaskBase::run() {
        std::shared_ptr<TileLayer> layer = _layer.lock();
        if (!layer) {
            return;
        }
            
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_canceled) {
                return;
            }
            _started = true;
        }
        
        bool refresh = loadTile(layer);
    
        layer->_fetchingTiles.remove(_tile.getTileId());

        if (refresh) {
            std::shared_ptr<MapRenderer> mapRenderer;
            {
                std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                mapRenderer = layer->_mapRenderer.lock();
            }
            if (mapRenderer) {
                mapRenderer->layerChanged(layer->shared_from_this(), false);
                mapRenderer->requestRedraw();
            }
        }
    }
    
    const float TileLayer::DISCRETE_ZOOM_LEVEL_BIAS = 0.001f;

    const float TileLayer::PRELOADING_TILE_SCALE = 2.0f;
    const float TileLayer::SUBDIVISION_THRESHOLD = Const::WORLD_SIZE;
    
}
