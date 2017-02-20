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

namespace carto {
    class TileDrawData;
    namespace vt {
        class Tile;
    }
    
    /**
     * A tile layer where each tile is a bitmap. Should be used together with corresponding data source.
     */
    class RasterTileLayer: public TileLayer {
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
    
    protected:
        class FetchTask : public TileLayer::FetchTaskBase {
        public:
            FetchTask(const std::shared_ptr<RasterTileLayer>& layer, const MapTile& tile, bool preloadingTile);
    
        protected:
            bool loadTile(const std::shared_ptr<TileLayer>& tileLayer);
            
        private:
            static std::shared_ptr<Bitmap> extractSubTile(const MapTile& subTile, const MapTile& tile, const std::shared_ptr<Bitmap>& bitmap);
            static std::shared_ptr<vt::Tile> createVectorTile(const MapTile& tile, const std::shared_ptr<Bitmap>& bitmap);
        };
    
        virtual bool tileExists(const MapTile& mapTile, bool preloadingCache) const;
        virtual bool tileValid(const MapTile& mapTile, bool preloadingCache) const;
        virtual void fetchTile(const MapTile& mapTile, bool preloadingTile, bool invalidated);
        virtual void clearTiles(bool preloadingTiles);
        virtual void tilesChanged(bool removeTiles);

        virtual void calculateDrawData(const MapTile& visTile, const MapTile& closestTile, bool preloadingTile);
        virtual void refreshDrawData(const std::shared_ptr<CullState>& cullState);
        
        virtual int getMinZoom() const;
        virtual int getMaxZoom() const;
        virtual std::vector<long long> getVisibleTileIds() const;
        
        virtual void offsetLayerHorizontally(double offset);
        
        virtual void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        virtual bool onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState);
        virtual bool onDrawFrame3D(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState);
        virtual void onSurfaceDestroyed();
        
        virtual void registerDataSourceListener();
        virtual void unregisterDataSourceListener();

    private:    
        static const int DEFAULT_CULL_DELAY = 200;
        static const int PRELOADING_PRIORITY_OFFSET = -2;
        static const int EXTRA_TILE_FOOTPRINT = 4096;
        static const int DEFAULT_PRELOADING_CACHE_SIZE = 10 * 1024 * 1024;
        
        std::vector<long long> _visibleTileIds;
        std::vector<std::shared_ptr<TileDrawData> > _tempDrawDatas;
        
        cache::timed_lru_cache<long long, std::shared_ptr<const vt::Tile> > _visibleCache;
        cache::timed_lru_cache<long long, std::shared_ptr<const vt::Tile> > _preloadingCache;
    };
    
}

#endif
