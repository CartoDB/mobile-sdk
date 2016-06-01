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
#include <unordered_map>

namespace carto {
    class CancelableTask;
    class CullState;
    class TileLoadListener;
    
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
         * Sets the zoom level bias for this layer. Higher zoom level bias forces SDK to use more detailed tiles for given view compared to lower zoom bias.
         * The default bias is 0.
         * @param bias The new bias value, both positive and negative fractional values are supported.
         */
        void setZoomLevelBias(float bias);
        
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
        virtual void clearTileCaches(bool all) = 0;

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
    
        virtual bool isUpdateInProgress() const;
        
    protected:
        class DataSourceListener : public TileDataSource::OnChangeListener {
        public:
            DataSourceListener(const std::shared_ptr<TileLayer>& layer);
            
            virtual void onTilesChanged(bool removeTiles);
            
        private:
            std::weak_ptr<TileLayer> _layer;
        };
        
        class FetchTaskBase : public CancelableTask {
        public:
            FetchTaskBase(const std::shared_ptr<TileLayer>& layer, const MapTile& tile, bool preloadingTile);
            
            bool isPreloading() const;
            bool isInvalidated() const;
            void invalidate();
            virtual void cancel();
            virtual void run();
            
        protected:
            virtual bool loadTile(const std::shared_ptr<TileLayer>& layer) = 0;
            
            std::weak_ptr<TileLayer> _layer;
            MapTile _tile; // original tile
            std::vector<MapTile> _dataSourceTiles; // tiles in valid datasource range, ordered to top

        private:
            bool _preloadingTile;
            bool _started;
            bool _invalidated;
        };
        
        class FetchingTileTasks {
        public:
            FetchingTileTasks() : _fetchingTiles(), _mutex() {}
            
            void add(long long tileId, const std::shared_ptr<FetchTaskBase>& task) {
                std::lock_guard<std::mutex> lock(_mutex);
                _fetchingTiles[tileId] = task;
            }
            
            bool exists(long long tileId) {
                std::lock_guard<std::mutex> lock(_mutex);
                return _fetchingTiles.find(tileId) != _fetchingTiles.end();
            }
            
            void remove(long long tileId) {
                std::lock_guard<std::mutex> lock(_mutex);
                _fetchingTiles.erase(tileId);
            }
            
            std::vector<std::shared_ptr<FetchTaskBase> > getTasks() const {
                std::lock_guard<std::mutex> lock(_mutex);
                std::vector<std::shared_ptr<FetchTaskBase> > tasks;
                for (const auto& pair : _fetchingTiles) {
                    tasks.push_back(pair.second);
                }
                return tasks;
            }
            
            int getPreloadingCount() const {
                std::lock_guard<std::mutex> lock(_mutex);
                int count = 0;
                for (const auto& pair : _fetchingTiles) {
                    if (pair.second->isPreloading()) {
                        count++;
                    }
                }
                return count;
            }
            
            int getVisibleCount() const {
                std::lock_guard<std::mutex> lock(_mutex);
                int count = 0;
                for (const auto& pair : _fetchingTiles) {
                    if (!pair.second->isPreloading()) {
                        count++;
                    }
                }
                return count;
            }

        private:
            std::unordered_map<long long, std::shared_ptr<FetchTaskBase> > _fetchingTiles;
            mutable std::mutex _mutex;
        };
        
        TileLayer(const std::shared_ptr<TileDataSource>& dataSource);
        
        virtual void loadData(const std::shared_ptr<CullState>& cullState);

        virtual void updateTileLoadListener();

        virtual bool tileExists(const MapTile& tile, bool preloadingCache) = 0;
        virtual bool tileIsValid(const MapTile& tile) const = 0;
        virtual void fetchTile(const MapTile& tile, bool preloadingTile, bool invalidated) = 0;

        MapBounds calculateInternalTileBounds(const MapTile& mapTile) const;
        virtual void calculateDrawData(const MapTile& visTile, const MapTile& closestTile, bool preloadingTile) = 0;
        virtual void refreshDrawData(const std::shared_ptr<CullState>& cullState) = 0;
        
        virtual void tilesChanged(bool removeTiles) = 0;
        
        virtual int getMinZoom() const = 0;
        virtual int getMaxZoom() const = 0;
        
        static const float DISCRETE_ZOOM_LEVEL_BIAS;

        std::atomic<bool> _synchronizedRefresh;

        std::atomic<bool> _calculatingTiles;
        std::atomic<bool> _refreshedTiles;
        
        const DirectorPtr<TileDataSource> _dataSource;
        std::shared_ptr<DataSourceListener> _dataSourceListener;
        
        DirectorPtr<TileLoadListener> _tileLoadListener;
        mutable std::mutex _tileLoadListenerMutex;
    
        FetchingTileTasks _fetchingTiles;
        
        int _frameNr;
        int _lastFrameNr;
    
        bool _preloading;
        
        TileSubstitutionPolicy::TileSubstitutionPolicy _substitutionPolicy;
    
        float _zoomLevelBias;
    
    private:
        void calculateVisibleTiles(const std::shared_ptr<CullState>& cullState);
        void calculateVisibleTilesRecursive(const std::shared_ptr<CullState>& cullState, const MapTile& mapTile);
        
        void findTiles(const std::vector<MapTile>& visTiles, bool preloadingTiles);
        bool findParentTile(const MapTile& visTile, const MapTile& tile, int depth, bool preloadingCache, bool preloadingTile);
        int findChildTiles(const MapTile& visTile, const MapTile& tile, int depth, bool preloadingCache, bool preloadingTile);
    
        static const int MAX_PARENT_SEARCH_DEPTH = 6;
        static const int MAX_CHILD_SEARCH_DEPTH = 3;
        
        static const float PRELOADING_TILE_SCALE;
        static const float SUBDIVISION_THRESHOLD;
        
        std::vector<MapTile> _visibleTiles;
        std::vector<MapTile> _preloadingTiles;
    };
    
}

#endif
