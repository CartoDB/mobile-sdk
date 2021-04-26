/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PERSISTENTCACHETILEDATASOURCE_H_
#define _CARTO_PERSISTENTCACHETILEDATASOURCE_H_

#include "core/MapBounds.h"
#include "components/CancelableThreadPool.h"
#include "components/DirectorPtr.h"
#include "datasources/CacheTileDataSource.h"

#include <string>

#include <stdext/timed_lru_cache.h>

namespace sqlite3pp {
    class database;
}

namespace carto {
    class TileDownloadListener;

    /**
     * A tile data source that loads tiles from another tile data source
     * and caches them in an offline sqlite database. Tiles will remain in the database
     * even after the application is closed.
     * The database contains table "persistent_cache" with the following fields:
     * "tileId" (tile id), "compressed" (compressed tile image),
     * "time" (the time the tile was cached in milliseconds from epoch).
     * Default cache capacity is 50MB.
     */
    class PersistentCacheTileDataSource : public CacheTileDataSource {
    public:
        /**
         * Constructs a PersistentCacheTileDataSource object from tile data source
         * and a sqlite database. The sqlite database must be writable, if it doesn't exist, an empty one
         * will be created instead.
         * @param dataSource The datasource to be cached.
         * @param databasePath The path to the sqlite database, where the tiles will be cached.
         */
        PersistentCacheTileDataSource(const std::shared_ptr<TileDataSource>& dataSource, const std::string& databasePath);
        virtual ~PersistentCacheTileDataSource();
        
        /**
         * Returns the state of cache only mode.
         * @return True when cache only mode is enabled, false otherwise.
         */
        bool isCacheOnlyMode() const;
        /**
         * Switches the datasource to 'cache only' mode or switches back from the mode.
         * If enabled, tiles are loaded from the cache only and the original data source is not used.
         * By default, cache only mode is off.
         * @param enabled True when the mode should be enabled, false otherwise.
         */
        void setCacheOnlyMode(bool enabled);

        /**
         * Starts downloading the specified area. The area will be stored in the cache.
         * Note that is the area is too big or cache is already filled, subsequent downloaded tiles
         * may push existing tile out of the cache.
         * @param mapBounds The bounds of the area to download. The coordinate system of the bounds must be the same as specified in the data source projection.
         * @param minZoom The minimum zoom of the tiles to load.
         * @param maxZoom The maximum zoom of the tiles to load.
         * @param tileDownloadListener The tile download listener to use that will receive download related callbacks.
         */
        void startDownloadArea(const MapBounds& mapBounds, int minZoom, int maxZoom, const std::shared_ptr<TileDownloadListener>& tileDownloadListener);
        /**
         * Stops all background downloader processes.
         */
        void stopAllDownloads();

        /**
         * Returns the status of the cache database.
         * @return True if the persistent cache database is open and working, false otherwise.
         */
        bool isOpen() const;

        /**
         * Closes the cache database. The datasource will still work afterwards,
         * but all requests will be directed to the original datasource.
         */
        void close();

        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);
        
        virtual void clear();
        
        virtual std::size_t getCapacity() const;

        virtual void setCapacity(std::size_t capacityInBytes);

    protected:
        class DownloadTask : public CancelableTask {
        public:
            DownloadTask(const std::shared_ptr<PersistentCacheTileDataSource>& dataSource, const MapBounds& mapBounds, int minZoom, int maxZoom, const std::shared_ptr<TileDownloadListener>& listener);
            
            virtual void run();
    
        private:
            std::weak_ptr<PersistentCacheTileDataSource> _dataSource;
            MapBounds _mapBounds;
            int _minZoom;
            int _maxZoom;
            DirectorPtr<TileDownloadListener> _downloadListener;
        };

        static const unsigned int DEFAULT_CAPACITY;
        static const unsigned int EXTRA_TILE_FOOTPRINT;

        void openDatabase(const std::string& databasePath);
        void closeDatabase();
        void loadTileInfo();

        void downloadArea(const MapBounds& mapBounds, int minZoom, int maxZoom, const std::shared_ptr<TileDownloadListener>& listener);
        
        std::shared_ptr<TileData> get(long long tileId);
        void store(long long tileId, const std::shared_ptr<TileData>& tileData);
        void remove(long long tileId);

        std::shared_ptr<long long> createTileId(long long tileId);
        
        std::unique_ptr<sqlite3pp::database> _database;
        
        bool _cacheOnlyMode;

        std::shared_ptr<CancelableThreadPool> _downloadThreadPool;
        
        cache::timed_lru_cache<long long, std::shared_ptr<long long> > _cache;
        mutable std::recursive_mutex _mutex;
    };

}

#endif
