/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_UTFGRIDRASTERTILELAYER_H_
#define _CARTO_UTFGRIDRASTERTILELAYER_H_

#include "RasterTileLayer.h"
#include "components/CancelableTask.h"
#include "components/Task.h"
#include "core/MapTile.h"

#include <memory>
#include <map>
#include <unordered_set>

#include <stdext/timed_lru_cache.h>

namespace carto {
    
    /**
     * A special raster layer with UTFGrid interaction. The layer needs
     * additional data source for UTF grid data and can used to query
     * nearest feature info (tooltips) around a specified point.
     */
    class UTFGridRasterTileLayer : public RasterTileLayer {
    public:
        UTFGridRasterTileLayer(const std::shared_ptr<TileDataSource>& rasterDataSource, const std::shared_ptr<TileDataSource>& utfGridDataSource);
        virtual ~UTFGridRasterTileLayer();
    
        virtual void clearTileCaches(bool all);
        
        /**
         * Locates and returns tooltips around the given map point.
         * @param mapPos The map position for the tooltips.
         * @param waitForData If true, then missing data is loaded and the call may block for several seconds. If false, then only cached/preloaded data is used.
         * @return The attribute map of tooltips.
         */
        std::map<std::string, std::string> getTooltips(const MapPos& mapPos, bool waitForData) const;
    
    protected:
        virtual bool tileExists(const MapTile& mapTile, bool preloadingCache);
        virtual bool tileIsValid(const MapTile& mapTile) const;
        virtual void fetchTile(const MapTile& mapTile, bool preloadingTile, bool invalidated);
    
        virtual void tilesChanged(bool removeTiles);
        
        virtual void registerDataSourceListener();
        virtual void unregisterDataSourceListener();
        
    private:
        class CombinedFetchTask : public RasterTileLayer::FetchTask {
        public:
            CombinedFetchTask(const std::shared_ptr<UTFGridRasterTileLayer>& layer, const MapTile& tile, bool preloadingTile);
            
        protected:
            virtual bool loadTile(const std::shared_ptr<TileLayer>& tileLayer);
        };

        class UTFGridTile {
        public:
            UTFGridTile(const std::vector<std::string>& keys, const std::map<std::string, std::map<std::string, std::string> >& data, const std::vector<int>& keyIds, int xSize, int ySize) : _keys(keys), _data(data), _keyIds(keyIds), _xSize(xSize), _ySize(ySize) { }
            
            std::string getKey(int keyId) const { return keyId >= 0 && keyId <= static_cast<int>(_keys.size()) ? _keys[keyId] : std::string(); }
            std::map<std::string, std::string> getData(const std::string& key) const { auto it = _data.find(key); return it != _data.end() ? it->second : std::map<std::string, std::string>(); }

            int getXSize() const { return _xSize; }
            int getYSize() const { return _ySize; }
            int getKeyId(int x, int y) const { return x >= 0 && y >= 0 && x < getXSize() && y < getYSize() ? _keyIds[y * getXSize() + x] : 0; }
            
            std::size_t getResidentSize() const { return sizeof(int) * _keyIds.size(); }

        private:
            std::vector<std::string> _keys;
            std::map<std::string, std::map<std::string, std::string> > _data;
            std::vector<int> _keyIds;
            int _xSize;
            int _ySize;
        };
        
        static std::shared_ptr<UTFGridTile> DecodeUTFTile(const TileData& tileData);
        
        static const int PRELOADING_PRIORITY_OFFSET = -2;
        static const int DEFAULT_PRELOADING_CACHE_SIZE = 10 * 1024 * 1024;

        const std::shared_ptr<TileDataSource> _utfGridDataSource;
    
        mutable cache::timed_lru_cache<long long, std::shared_ptr<UTFGridTile> > _utfGridVisibleCache;
        mutable cache::timed_lru_cache<long long, std::shared_ptr<UTFGridTile> > _utfGridPreloadingCache;
    };
    
}

#endif
