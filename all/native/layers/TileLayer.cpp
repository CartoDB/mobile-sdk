#include "TileLayer.h"
#include "core/BinaryData.h"
#include "components/CancelableTask.h"
#include "datasources/components/TileData.h"
#include "layers/TileLoadListener.h"
#include "layers/UTFGridEventListener.h"
#include "layers/components/UTFGridTile.h"
#include "renderers/components/CullState.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/MapRenderer.h"
#include "projections/Projection.h"
#include "ui/UTFGridClickInfo.h"
#include "utils/Const.h"
#include "utils/GeomUtils.h"
#include "utils/Log.h"

namespace carto {

    TileLayer::~TileLayer() {
    }
    
    std::shared_ptr<TileDataSource> TileLayer::getDataSource() const {
        return _dataSource.get();
    }
    
    std::shared_ptr<TileDataSource> TileLayer::getUTFGridDataSource() const {
        return _utfGridDataSource.get();
    }
    
    void TileLayer::setUTFGridDataSource(const std::shared_ptr<TileDataSource>& dataSource) {
        _utfGridDataSource.set(dataSource);
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
    
    void TileLayer::clearTileCaches(bool all) {
        clearTiles(true);
        if (all) {
            clearTiles(false);
        }
    }

    std::shared_ptr<TileLoadListener> TileLayer::getTileLoadListener() const {
        return _tileLoadListener.get();
    }
    
    void TileLayer::setTileLoadListener(const std::shared_ptr<TileLoadListener>& tileLoadListener) {
        _tileLoadListener.set(tileLoadListener);
    }

    std::shared_ptr<UTFGridEventListener> TileLayer::getUTFGridEventListener() const {
        return _utfGridEventListener.get();
    }
    
    void TileLayer::setUTFGridEventListener(const std::shared_ptr<UTFGridEventListener>& utfGridEventListener) {
        _utfGridEventListener.set(utfGridEventListener);
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
        _utfGridDataSource(),
        _tileLoadListener(),
        _utfGridEventListener(),
        _fetchingTiles(),
        _frameNr(0),
        _lastFrameNr(-1),
        _preloading(false),
        _substitutionPolicy(TileSubstitutionPolicy::TILE_SUBSTITUTION_POLICY_ALL),
        _zoomLevelBias(0.0f),
        _visibleTiles(),
        _preloadingTiles(),
        _utfGridTiles()
    {
    }
    
    void TileLayer::loadData(const std::shared_ptr<CullState>& cullState) {
        // This method (from update() or refresh()) might be called from multiple threads
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        _calculatingTiles = true;

        // Remove UTF grid tiles that are missing from the cache
        for (auto it = _utfGridTiles.begin(); it != _utfGridTiles.end(); ) {
            if (!tileExists(it->first, false) && !tileExists(it->first, true)) {
                it = _utfGridTiles.erase(it);
            }
            else {
                it++;
            }
        }
    
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
    
        DirectorPtr<TileLoadListener> tileLoadListener = _tileLoadListener;

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
    
    void TileLayer::calculateRayIntersectedElements(const Projection& projection, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        DirectorPtr<TileDataSource> utfGridDataSource = _utfGridDataSource;

        if (!utfGridDataSource) {
            return;
        }

        double t = 0;
        if (!cglib::intersect_plane(cglib::vec4<double>(0, 0, 1, 0), ray, &t)) {
            return;
        }

        MapPos mapPosInternal(ray(t)(0), ray(t)(1), ray(t)(2));
        MapPos mapPos = utfGridDataSource->getProjection()->fromInternal(mapPosInternal);
        int zoom = std::min(getMaxZoom(), static_cast<int>(viewState.getZoom() + getZoomLevelBias() + DISCRETE_ZOOM_LEVEL_BIAS));

        MapTile mapTile = calculateMapTile(mapPos, std::min(utfGridDataSource->getMaxZoom(), std::max(utfGridDataSource->getMinZoom(), zoom)));

        double tileWidth = utfGridDataSource->getProjection()->getBounds().getDelta().getX() / (1 << mapTile.getZoom());
        double tileHeight = utfGridDataSource->getProjection()->getBounds().getDelta().getY() / (1 << mapTile.getZoom());

        MapVec mapVec(mapTile.getX() * tileWidth, mapTile.getY() * tileHeight);
        MapPos mapTileOrigin = utfGridDataSource->getProjection()->getBounds().getMin() + mapVec;
        double xRel = (mapPos.getX() - mapTileOrigin.getX()) / tileWidth;
        double yRel = 1 - (mapPos.getY() - mapTileOrigin.getY()) / tileHeight;
        
        // Try to get the tile from cache
        std::shared_ptr<UTFGridTile> utfGridTile;
        for (MapTile flippedMapTile = mapTile.getFlipped(); !utfGridTile; flippedMapTile = flippedMapTile.getParent()) {
            if (tileExists(flippedMapTile, false) || tileExists(flippedMapTile, true)) {
                std::lock_guard<std::recursive_mutex> lock(_mutex);
                auto it = _utfGridTiles.find(flippedMapTile);
                if (it != _utfGridTiles.end()) {
                    utfGridTile = it->second;
                    break;
                }
            }
            if (flippedMapTile.getZoom() == 0) {
                break;
            }
        }

        // If succeeded and valid key under the click position, call the listener
        if (utfGridTile) {
            int x = static_cast<int>(std::floor(xRel * utfGridTile->getXSize()));
            int y = static_cast<int>(std::floor(yRel * utfGridTile->getYSize()));
            int keyId = utfGridTile->getKeyId(x, y);
            if (keyId != 0) {
                auto elementInfo = std::make_shared<std::map<std::string, std::string> >(utfGridTile->getData(utfGridTile->getKey(keyId)));
                std::shared_ptr<Layer> thisLayer = std::const_pointer_cast<Layer>(shared_from_this());
                results.push_back(RayIntersectedElement(elementInfo, thisLayer, mapPos, mapPos, 0));
            }
        }
    }

    bool TileLayer::processClick(ClickType::ClickType clickType, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const {
        DirectorPtr<UTFGridEventListener> utfGridEventListener = _utfGridEventListener;

        if (utfGridEventListener) {
            std::shared_ptr<std::map<std::string, std::string> > elementInfo = intersectedElement.getElement<std::map<std::string, std::string> >();
            auto utfGridClickInfo = std::make_shared<UTFGridClickInfo>(clickType, intersectedElement.getHitPos(), *elementInfo, intersectedElement.getLayer());
            return utfGridEventListener->onUTFGridClicked(utfGridClickInfo);
        }

        return clickType == ClickType::CLICK_TYPE_SINGLE || clickType == ClickType::CLICK_TYPE_LONG; // by default, disable 'click through' for single and long clicks
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
                for (int i = 1; i <= 5; i++) {
                    calculateVisibleTilesRecursive(cullState, MapTile(-i, 0, 0, _frameNr));
                    calculateVisibleTilesRecursive(cullState, MapTile( i, 0, 0, _frameNr));
                }
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
                if (!tileValid(tile, preloadingTiles) && !tileValid(tile, !preloadingTiles)) {
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
        if (refresh) {
            loadUTFGridTile(layer);
        }
    
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
    
    bool TileLayer::FetchTaskBase::loadUTFGridTile(const std::shared_ptr<TileLayer>& tileLayer) {
        DirectorPtr<TileDataSource> dataSource = tileLayer->_utfGridDataSource;

        std::vector<MapTile> dataSourceTiles;
        if (dataSource) {
            for (MapTile dataSourceTile = _tile; true; ) {
                int zoom = dataSourceTile.getZoom();
                if (zoom >= dataSource->getMinZoom() && zoom <= dataSource->getMaxZoom()) {
                    dataSourceTiles.push_back(dataSourceTile);
                }
                if (zoom <= 0) {
                    break;
                }
                dataSourceTile = dataSourceTile.getParent();
            }
        }

        bool refresh = false;
        for (const MapTile& dataSourceTile : dataSourceTiles) {
            std::shared_ptr<TileData> tileData = dataSource->loadTile(dataSourceTile);
            if (!tileData) {
                break;
            }
            if (tileData->isReplaceWithParent()) {
                continue;
            }

            std::shared_ptr<UTFGridTile> utfTile = UTFGridTile::DecodeUTFTile(tileData->getData());
            if (utfTile) {
                std::lock_guard<std::recursive_mutex> lock(tileLayer->_mutex);
                tileLayer->_utfGridTiles[dataSourceTile] = utfTile; // we ignore expiration info here
                refresh = true;
            }
            else {
                Log::Error("TileLayer::FetchTaskBase: Failed to decode UTF grid tile");
            }
            break;
        }
        return refresh;
    }

    const float TileLayer::DISCRETE_ZOOM_LEVEL_BIAS = 0.001f;

    const float TileLayer::PRELOADING_TILE_SCALE = 2.0f;
    const float TileLayer::SUBDIVISION_THRESHOLD = Const::WORLD_SIZE;
    
}
