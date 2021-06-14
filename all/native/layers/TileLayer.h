/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TILELAYER_H_
#define _CARTO_TILELAYER_H_

#include "core/MapPos.h"
#include "core/MapBounds.h"
#include "core/MapTile.h"
#include "components/CancelableTask.h"
#include "components/DirectorPtr.h"
#include "datasources/TileDataSource.h"
#include "layers/Layer.h"

#include <atomic>
#include <mutex>
#include <unordered_map>

namespace carto {
    class CancelableTask;
    class CullState;
    class GLResourceManager;
    class ProjectionSurface;
    class TileRenderer;
    class TileLoadListener;
    class UTFGridTile;
    class UTFGridEventListener;
    namespace vt {
        class TileTransformer;
    }
    
    namespace TileSubstitutionPolicy {
        /**
         * The policy to use when looking for tiles that are not available.
         */
        enum TileSubstitutionPolicy {
            /**
             * Consider all cached/loaded tiles.
             */
            TILE_SUBSTITUTION_POLICY_ALL,
            /**
             * Consider only tiles that are currently visible.
             * This is recommended for low-latency data sources, like offline sources.
             */
            TILE_SUBSTITUTION_POLICY_VISIBLE,
            /**
             * Never substitute tiles.
             */
            TILE_SUBSTITUTION_POLICY_NONE
        };
    }
        
    /**
     * An abstract base class for all tile layers.
     */
    class TileLayer : public Layer {
    public:
        virtual ~TileLayer();
        
        /**
         * Returns the data source assigned to this layer.
         * @return The tile data source assigned to this layer.
         */
        std::shared_ptr<TileDataSource> getDataSource() const;

        /**
         * Returns the tile data source of the associated UTF grid. By default this is null.
         * @return The tile data source of the associated UTF grid.
         */
        std::shared_ptr<TileDataSource> getUTFGridDataSource() const;
        /**
         * Sets the tile data source of the associated UTF grid.
         * @param dataSource The data source to use. Can be null if UTF grid is not used.
         */
        void setUTFGridDataSource(const std::shared_ptr<TileDataSource>& dataSource);
    
        /**
         * Returns the current frame number.
         * @return The current frame number.
         */
        int getFrameNr() const;
        /**
         * Sets the frame number, only used for animated tiles. 
         * Loading a new frame may take some time, previous frame is shown during loading.
         * @param frameNr The frame number to display.
         */
        void setFrameNr(int frameNr);
    
        /**
         * Returns the state of the preloading flag of this layer.
         * @return True if preloading is enabled.
         */
        bool isPreloading() const;
        /**
         * Sets the state of preloading for this layer. Preloading allows the downloading of tiles that are not
         * currently visible on screen, but are adjacent to ones that are. This means that the user can pan the map without
         * immediately noticing any missing tiles.
         *
         * Enabling this option might introduce a small performance hit on slower devices. It should also be noted that this
         * will considerably increase network traffic if used with online maps. The default is false.
         * @param preloading The new preloading state of the layer.
         */
        void setPreloading(bool preloading);
        
        /**
         * Returns the state of the synchronized refresh flag.
         * @return The state of the synchronized refresh flag.
         */
        bool isSynchronizedRefresh() const;
        /**
         * Sets the state of the synchronized refresh flag. If disabled all tiles will appear on screen
         * one by one as they finish loading. If enabled the map will wait for all the visible tiles to finish loading
         * and then show them all on screen together. This is useful for animated tiles.
         * @param synchronizedRefresh The new state of the synchronized refresh flag.
         */
        void setSynchronizedRefresh(bool synchronizedRefresh);
    
        /**
         * Returns the current tile substitution policy.
         * @return The current substitution policy. Default is TILE_SUBSTITUTION_POLICY_ALL.
         */
        TileSubstitutionPolicy::TileSubstitutionPolicy getTileSubstitutionPolicy() const;
        /**
         * Sets the current tile substitution policy.
         * @param policy The new substitution policy. Default is TILE_SUBSTITUTION_POLICY_ALL.
         */
        void setTileSubstitutionPolicy(TileSubstitutionPolicy::TileSubstitutionPolicy policy);
        
        /**
         * Gets the current zoom level bias for this layer.
         * @return The current zoom level bias for this layer.
         */
        float getZoomLevelBias() const;
        /**
         * Sets the zoom level bias for this layer.
         * Higher zoom level bias forces SDK to use more detailed tiles for given view compared to lower zoom bias.
         * The default bias is 0.
         * @param bias The new bias value, both positive and negative fractional values are supported.
         */
        void setZoomLevelBias(float bias);
        
        /**
         * Gets the current maximum overzoom level for this layer.
         * @return The current maximum overzoom level for this layer.
         */
        int getMaxOverzoomLevel() const;
        /**
         * Sets the maximum overzoom level for this layer.
         * If a tile for the given zoom level Z is not available, SDK will try to use tiles with zoom levels Z-1, ..., Z-MaxOverzoomLevel.
         * The default is 6.
         * @param overzoomLevel The new maximum overzoom value.
         */
        void setMaxOverzoomLevel(int overzoomLevel);
        
        /**
         * Gets the current maximum underzoom level for this layer.
         * @return The current maximum underzoom level for this layer.
         */
        int getMaxUnderzoomLevel() const;

        /**
         * Sets the maximum underzoom level for this layer.
         * If a tile for the given zoom level Z is not available, SDK will try to use tiles with zoom levels Z-1, ..., Z-MaxOverzoomLevel and then Z+1, ..., Z+MaxUnderzoomLevel.
         * The default is 3.
         * @param underzoomLevel The new maximum underzoom value.
         */
        void setMaxUnderzoomLevel(int underzoomLevel);
        
        /**
         * Calculates the tile corresponding to given geographical coordinates and zoom level.
         * Note: zoom level bias is NOT applied, only discrete zoom level is used.
         * @param mapPos Coordinates of the point in data source projection coordinate system.
         * @param zoom Zoom level to use for the tile.
         * @return The corresponding map tile.
         */
        MapTile calculateMapTile(const MapPos& mapPos, int zoom) const;
        /**
         * Calculates the origin of given map tile.
         * @param mapTile The map tile to use.
         * @return The corresponding coordinates of the tile origin in data source projection coordinate system.
         */
        MapPos calculateMapTileOrigin(const MapTile& mapTile) const;
        /**
         * Calculates the bounds of given map tile.
         * @param mapTile The map tile to use.
         * @return The corresponding bounds of the tile origin in data source projection coordinate system.
         */
        MapBounds calculateMapTileBounds(const MapTile& mapTile) const;

        /**
         * Clears layer tile caches. This will release memory allocated to tiles.
         * @param all True if all tiles should be released, otherwise only preloading (invisible) tiles are released.
         */
        void clearTileCaches(bool all);

        /**
         * Returns the tile load listener.
         * @return The tile load listener.
         */
        std::shared_ptr<TileLoadListener> getTileLoadListener() const;
        /**
         * Sets the tile load listener.
         * @param tileLoadListener The tile load listener.
         */
        void setTileLoadListener(const std::shared_ptr<TileLoadListener>& tileLoadListener);

        /**
         * Returns the UTF grid event listener.
         * @return The UTF grid event listener.
         */
        std::shared_ptr<UTFGridEventListener> getUTFGridEventListener() const;
        /**
         * Sets the UTF grid event listener.
         * @param utfGridEventListener The UTF grid event listener.
         */
        void setUTFGridEventListener(const std::shared_ptr<UTFGridEventListener>& utfGridEventListener);
    
        virtual bool isUpdateInProgress() const;
        
    protected:
        class DataSourceListener : public TileDataSource::OnChangeListener {
        public:
            explicit DataSourceListener(const std::shared_ptr<TileLayer>& layer);
            
            virtual void onTilesChanged(bool removeTiles);
            
        private:
            std::weak_ptr<TileLayer> _layer;
        };
        
        class FetchTaskBase : public CancelableTask {
        public:
            FetchTaskBase(const std::shared_ptr<TileLayer>& layer, long long tileId, const MapTile& tile, bool preloadingTile);
            
            long long getTileId() const;
            MapTile getMapTile() const;
            bool isPreloadingTile() const;

            bool isInvalidated() const;
            void invalidate();

            virtual void cancel();
            virtual void run();
            
        protected:
            virtual bool loadTile(const std::shared_ptr<TileLayer>& layer) = 0;
            
            std::weak_ptr<TileLayer> _layer;
            long long _tileId;
            MapTile _tile; // original tile
            bool _preloadingTile;
            std::vector<MapTile> _dataSourceTiles; // tiles in valid datasource range, ordered to top

        private:
            bool loadUTFGridTile(const std::shared_ptr<TileLayer>& layer);

            bool _started;
            std::atomic<bool> _invalidated;
        };
        
        class FetchingTileTasks {
        public:
            FetchingTileTasks() : _fetchingTiles(), _mutex() { }
            
            std::vector<std::shared_ptr<FetchTaskBase> > get(long long tileId) const {
                std::lock_guard<std::mutex> lock(_mutex);
                auto it = _fetchingTiles.find(tileId);
                return it != _fetchingTiles.end() ? it->second : std::vector<std::shared_ptr<FetchTaskBase> >();
            }
            
            void insert(long long tileId, const std::shared_ptr<FetchTaskBase>& task) {
                std::lock_guard<std::mutex> lock(_mutex);
                _fetchingTiles[tileId].push_back(task);
            }
            
            void remove(long long tileId, const std::shared_ptr<FetchTaskBase>& task) {
                std::lock_guard<std::mutex> lock(_mutex);
                auto it = _fetchingTiles.find(tileId);
                if (it == _fetchingTiles.end()) {
                    return;
                }
                std::vector<std::shared_ptr<FetchTaskBase> >& tasks = it->second;
                auto it2 = std::find(tasks.begin(), tasks.end(), task);
                if (it2 == tasks.end()) {
                    return;
                }
                tasks.erase(it2);
                if (tasks.empty()) {
                    _fetchingTiles.erase(it);
                }
            }
            
            std::vector<std::shared_ptr<FetchTaskBase> > getAll() const {
                std::lock_guard<std::mutex> lock(_mutex);
                std::vector<std::shared_ptr<FetchTaskBase> > tasks;
                for (auto it = _fetchingTiles.begin(); it != _fetchingTiles.end(); it++) {
                    tasks.insert(tasks.end(), it->second.begin(), it->second.end());
                }
                return tasks;
            }
            
            int getPreloadingCount() const {
                std::lock_guard<std::mutex> lock(_mutex);
                int count = 0;
                for (auto it = _fetchingTiles.begin(); it != _fetchingTiles.end(); it++) {
                    for (const std::shared_ptr<FetchTaskBase>& task : it->second) {
                        if (task->isPreloadingTile()) {
                            count++;
                        }
                    }
                }
                return count;
            }
            
            int getVisibleCount() const {
                std::lock_guard<std::mutex> lock(_mutex);
                int count = 0;
                for (auto it = _fetchingTiles.begin(); it != _fetchingTiles.end(); it++) {
                    for (const std::shared_ptr<FetchTaskBase>& task : it->second) {
                        if (!task->isPreloadingTile()) {
                            count++;
                        }
                    }
                }
                return count;
            }

        private:
            std::unordered_map<long long, std::vector<std::shared_ptr<FetchTaskBase> > > _fetchingTiles;
            mutable std::mutex _mutex;
        };

        explicit TileLayer(const std::shared_ptr<TileDataSource>& dataSource);

        virtual void setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                                   const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                                   const std::weak_ptr<Options>& options,
                                   const std::weak_ptr<MapRenderer>& mapRenderer,
                                   const std::weak_ptr<TouchHandler>& touchHandler);

        virtual void loadData(const std::shared_ptr<CullState>& cullState);

        virtual void updateTileLoadListener();

        virtual long long getTileId(const MapTile& tile) const = 0;
        virtual bool tileExists(long long tileId, bool preloadingCache) const = 0;
        virtual bool tileValid(long long tileId, bool preloadingCache) const = 0;
        virtual bool prefetchTile(long long tileId, bool preloadingTile) = 0;
        virtual void fetchTile(long long tileId, const MapTile& mapTile, bool preloadingTile, int priorityDelta) = 0;
        virtual void clearTiles(bool preloadingTiles) = 0;
        virtual void tilesChanged(bool removeTiles) = 0;

        virtual void calculateDrawData(const MapTile& visTile, const MapTile& closestTile, bool preloadingTile) = 0;
        virtual void refreshDrawData(const std::shared_ptr<CullState>& cullState) = 0;
        
        virtual int getMinZoom() const = 0;
        virtual int getMaxZoom() const = 0;
        virtual std::vector<long long> getVisibleTileIds() const = 0;
        
        virtual void calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
        virtual bool processClick(ClickType::ClickType clickType, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const;

        MapBounds calculateInternalTileBounds(const MapTile& mapTile) const;

        std::shared_ptr<vt::TileTransformer> getTileTransformer() const;
        void resetTileTransformer();

        const DirectorPtr<TileDataSource> _dataSource;
        std::shared_ptr<DataSourceListener> _dataSourceListener;

        std::shared_ptr<TileRenderer> _tileRenderer;
    
        FetchingTileTasks _fetchingTileTasks;
        
    private:
        struct FetchTileInfo {
            MapTile tile;
            bool preloading;
            int priorityDelta;
        };

        void calculateVisibleTiles(const std::shared_ptr<CullState>& cullState);
        void calculateVisibleTilesRecursive(const std::shared_ptr<CullState>& cullState, const MapTile& mapTile, const MapBounds& dataExtent);

        void sortTiles(std::vector<MapTile>& tiles, const ViewState& viewState, bool preloadingTiles);
        void buildFetchTiles(const std::vector<MapTile>& visTiles, bool preloadingTiles, std::vector<FetchTileInfo>& fetchTileList);

        bool findParentTile(const MapTile& visTile, const MapTile& tile, int depth, bool preloadingCache, bool preloadingTile);
        int findChildTiles(const MapTile& visTile, const MapTile& tile, int depth, bool preloadingCache, bool preloadingTile);

        static const float DISCRETE_ZOOM_LEVEL_BIAS;

        static const int MAX_PARENT_SEARCH_DEPTH;
        static const int MAX_CHILD_SEARCH_DEPTH;

        static const int PARENT_PRIORITY_OFFSET;
        static const int PRELOADING_PRIORITY_OFFSET;
        static const double PRELOADING_TILE_SCALE;
        static const float SUBDIVISION_THRESHOLD;
        
        std::atomic<bool> _calculatingTiles;
        std::atomic<bool> _refreshedTiles;
        
        ThreadSafeDirectorPtr<TileDataSource> _utfGridDataSource;
        
        ThreadSafeDirectorPtr<TileLoadListener> _tileLoadListener;
    
        ThreadSafeDirectorPtr<UTFGridEventListener> _utfGridEventListener;

        std::atomic<bool> _synchronizedRefresh;

        int _frameNr;
        int _lastFrameNr;
    
        bool _preloading;
        
        TileSubstitutionPolicy::TileSubstitutionPolicy _substitutionPolicy;
    
        float _zoomLevelBias;
        int _maxOverzoomLevel;
        int _maxUnderzoomLevel;

        std::vector<MapTile> _visibleTiles;
        std::vector<MapTile> _preloadingTiles;
        std::unordered_map<MapTile, std::shared_ptr<UTFGridTile> > _utfGridTiles;

        std::weak_ptr<GLResourceManager> _glResourceManager;
        std::weak_ptr<ProjectionSurface> _projectionSurface;
    };
    
}

#endif
