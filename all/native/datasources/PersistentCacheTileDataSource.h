/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PERSISTENTCACHETILEDATASOURCE_H_
#define _CARTO_PERSISTENTCACHETILEDATASOURCE_H_

#include "datasources/CacheTileDataSource.h"

#include <list>
#include <string>
#include <unordered_map>

namespace sqlite3pp {
    class database;
}

namespace carto {

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
         * Close the cache database. The datasource will still work afterwards,
         * but all requests will be directed to the original datasource.
         */
        virtual void close();

        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);
        
        virtual void clear();
        
        virtual std::size_t getCapacity() const;

        virtual void setCapacity(std::size_t capacity);

    protected:
        struct CacheElement {
            CacheElement(long long tileId, size_t sizeInBytes);
            long long _tileId;
            size_t _sizeInBytes;
        };
        
        typedef std::list<CacheElement> CacheElementList;
        typedef std::unordered_map<long long, CacheElementList::iterator> CacheElementItMap;
        
        void openDatabase(const std::string& databasePath);
        void closeDatabase();
        
        void loadTileSet();
        
        void removeOldestElements();
        
        std::shared_ptr<TileData> get(long long tileId);
        
        void remove(long long tileId);
        void removeAll();
        
        void store(long long tileId, const std::shared_ptr<TileData>& tileBitmap);
        
        std::unique_ptr<sqlite3pp::database> _database;
        
        bool _cacheOnlyMode;
        std::size_t _capacityInBytes;
        std::size_t _sizeInBytes;
        
        CacheElementList _lruElements;
        CacheElementItMap _mappedElements;
        
        mutable std::recursive_mutex _mutex;
    };

}

#endif
