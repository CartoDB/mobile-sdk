/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_RASTERTILELAYER_H_
#define _CARTO_RASTERTILELAYER_H_

#include "core/MapTile.h"
#include "components/CancelableTask.h"
#include "components/DirectorPtr.h"
#include "components/Task.h"
#include "layers/TileLayer.h"

#include <atomic>
#include <memory>
#include <map>

#include <stdext/timed_lru_cache.h>

#include <vt/Styles.h>

namespace carto {
    class TileDrawData;
    class RasterTileEventListener;
    namespace vt {
        class Tile;
    }
    
    namespace RasterTileFilterMode {
        /**
         * Supported raster tile filtering modes.
         */
        enum RasterTileFilterMode {
            /**
             * No filter (nearest texel).
             */
            RASTER_TILE_FILTER_MODE_NEAREST,
            /**
             * Bilinear filter (interpolate between 4 closest texels).
             */
            RASTER_TILE_FILTER_MODE_BILINEAR,
            /**
             * Bicubic filter (interpolate between 16 closest texels).
             */
            RASTER_TILE_FILTER_MODE_BICUBIC
        };
    }

    /**
     * A tile layer where each tile is a bitmap. Should be used together with corresponding data source.
     */
    class RasterTileLayer : public TileLayer {
    public:
        /**
         * Constructs a RasterTileLayer object from a data source.
         * @param dataSource The data source from which this layer loads data.
         */
        explicit RasterTileLayer(const std::shared_ptr<TileDataSource>& dataSource);
        virtual ~RasterTileLayer();
        
        /**
         * Returns the tile texture cache capacity.
         * @return The tile texture cache capacity in bytes.
         */
        std::size_t getTextureCacheCapacity() const;
        /**
         * Sets the tile texture cache capacity. Texture cache is the primary storage for raster data,
         * all tiles contained within the texture cache are stored as uncompressed openGL textures and can immediately be
         * drawn to the screen. Setting the cache size too small may cause artifacts, such as disappearing tiles.
         * The more tiles are visible on the screen, the larger this cache should be. A single opaque 256x256 tile takes
         * up 192KB of memory, a transparent tile of the same size takes 256KB. The number of tiles on the screen depends
         * on the screen size and density, current rotation and tilt angle, tile draw size parameter and 
         * whether or not preloading is enabled.
         * The default is 10MB, which should be enough for most use cases with preloading enabled. If preloading is
         * disabled, the cache size should be reduced by the user to conserve memory.
         * @param capacityInBytes The new tile bitmap cache capacity in bytes.
         */
        void setTextureCacheCapacity(std::size_t capacityInBytes);
    
        /**
         * Returns the current tile filter mode.
         * @return The current tile filter mode. The default is bilinear.
         */
        RasterTileFilterMode::RasterTileFilterMode getTileFilterMode() const;
        /**
         * Sets the current tile filter mode.
         * @param filterMode The new tile filter mode.
         */
        void setTileFilterMode(RasterTileFilterMode::RasterTileFilterMode filterMode);

        /**
         * Returns the raster tile event listener.
         * @return The raster tile event listener.
         */
        std::shared_ptr<RasterTileEventListener> getRasterTileEventListener() const;
        /**
         * Sets the raster tile event listener.
         * @param eventListener The raster tile event listener.
         */
        void setRasterTileEventListener(const std::shared_ptr<RasterTileEventListener>& eventListener);
    
    protected:
        class FetchTask : public TileLayer::FetchTaskBase {
        public:
            FetchTask(const std::shared_ptr<RasterTileLayer>& layer, const MapTile& tile, bool preloadingTile);
    
        protected:
            bool loadTile(const std::shared_ptr<TileLayer>& tileLayer);
            
        private:
            static std::shared_ptr<Bitmap> ExtractSubTile(const MapTile& subTile, const MapTile& tile, const std::shared_ptr<Bitmap>& bitmap);
        };
    
        virtual bool tileExists(const MapTile& mapTile, bool preloadingCache) const;
        virtual bool tileValid(const MapTile& mapTile, bool preloadingCache) const;
        virtual void fetchTile(const MapTile& mapTile, bool preloadingTile, bool invalidated);
        virtual void clearTiles(bool preloadingTiles);
        virtual void tilesChanged(bool removeTiles);

        virtual vt::RasterFilterMode getRasterFilterMode() const;

        virtual std::shared_ptr<vt::Tile> createVectorTile(const MapTile& tile, const std::shared_ptr<Bitmap>& bitmap) const;

        virtual void calculateDrawData(const MapTile& visTile, const MapTile& closestTile, bool preloadingTile);
        virtual void refreshDrawData(const std::shared_ptr<CullState>& cullState);
        
        virtual int getMinZoom() const;
        virtual int getMaxZoom() const;
        virtual std::vector<long long> getVisibleTileIds() const;
        
        virtual void calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
        virtual bool processClick(ClickType::ClickType clickType, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const;

        virtual void offsetLayerHorizontally(double offset);
        
        virtual bool onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState);
        virtual bool onDrawFrame3D(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState);
        
        virtual void registerDataSourceListener();
        virtual void unregisterDataSourceListener();

        RasterTileFilterMode::RasterTileFilterMode _tileFilterMode;

    private:    
        static const int DEFAULT_CULL_DELAY;
        static const int PRELOADING_PRIORITY_OFFSET;

        static const unsigned int EXTRA_TILE_FOOTPRINT;
        static const unsigned int DEFAULT_PRELOADING_CACHE_SIZE;
        
        ThreadSafeDirectorPtr<RasterTileEventListener> _rasterTileEventListener;

        std::vector<long long> _visibleTileIds;
        std::vector<std::shared_ptr<TileDrawData> > _tempDrawDatas;
        
        cache::timed_lru_cache<long long, std::shared_ptr<const vt::Tile> > _visibleCache;
        cache::timed_lru_cache<long long, std::shared_ptr<const vt::Tile> > _preloadingCache;
    };
    
}

#endif
