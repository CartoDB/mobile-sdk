/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOONLINETILEDATASOURCE_H_
#define _CARTO_CARTOONLINETILEDATASOURCE_H_

#include "datasources/TileDataSource.h"
#include "network/HTTPClient.h"

#include <random>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace carto {
    class BinaryData;
    class PackageTileMask;
    
    /**
     * An online tile data source that connects to Carto tile server.
     * This data source can be used with both vector and raster tiles,
     * depending on the specified source id.
     */
    class CartoOnlineTileDataSource : public TileDataSource {
    public:
        /**
         * Constructs a CartoOnlineTileDataSource object.
         * @param source Tile source id. For example "carto.streets".
         */
        explicit CartoOnlineTileDataSource(const std::string& source);
        virtual ~CartoOnlineTileDataSource();

        /**
         * Returns the current schema of the datasource.
         * @return The current schema of the datasource.
         */
        std::string getSchema();

        /**
         * Returns the current timeout value.
         * @return The current timeout value in seconds. If negative, then default platform-specific timeout is used.
         */
        int getTimeout() const;
        /**
         * Sets the current timeout value.
         * @param timeout The new timeout value in seconds. If negative, then default platform-specific timeout is used.
         */
        void setTimeout(int timeout);

        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);
        
    protected:
        struct TileMask {
            bool inclusive = true;
            std::shared_ptr<PackageTileMask> tileMask;
            std::shared_ptr<TileData> tileData;
        };

        struct CacheRecord {
            std::chrono::steady_clock::time_point lastAccessTime;
            std::shared_ptr<TileData> tileData;
        };

        std::string buildTileURL(const std::string& baseURL, const MapTile& tile) const;

        bool loadConfiguration();

        std::shared_ptr<TileData> loadOnlineTile(const std::string& url, const MapTile& mapTile);

        std::shared_ptr<TileData> loadCacheTile(const MapTile& mapTile);
        void removeCacheTile(const MapTile& mapTile);
        void storeCacheTile(const MapTile& mapTile, const std::shared_ptr<TileData>& tileData);

        static const int DEFAULT_MAX_ZOOM;
        static const int CACHE_MAX_ZOOM_DELTA ;
        static const float CACHE_ZOOM_WEIGHT_FACTOR;
        static const unsigned int MAX_CACHED_TILES;
        static const std::size_t MAX_CACHE_SIZE;
        static const std::string TILE_SERVICE_TEMPLATE;

        const std::string _source;
        mutable std::unordered_map<MapTile, CacheRecord> _cacheRecords;
        mutable std::size_t _cacheSize;
        HTTPClient _httpClient;

        std::string _schema;
        int _timeout;

        bool _tmsScheme;
        std::vector<std::string> _tileURLs;
        std::vector<TileMask> _tileMasks;
        std::default_random_engine _randomGenerator;

        mutable std::recursive_mutex _mutex;
    };
    
}

#endif
