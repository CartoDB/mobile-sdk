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

#include <stdext/timed_lru_cache.h>

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

        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);
        
    protected:
        struct TileMask {
            bool inclusive = true;
            std::shared_ptr<PackageTileMask> tileMask;
            std::shared_ptr<BinaryData> tileData;
        };

        std::string buildTileURL(const std::string& baseURL, const MapTile& tile) const;

        bool loadConfiguration();

        std::shared_ptr<TileData> loadOnlineTile(const std::string& url, const MapTile& mapTile);

        static const int DEFAULT_MAX_ZOOM = 14;
        static const int MAX_CACHED_TILES = 8;
        static const std::string TILE_SERVICE_TEMPLATE;

        const std::string _source;
        mutable cache::timed_lru_cache<long long, std::shared_ptr<TileData> > _cache;
        HTTPClient _httpClient;

        std::string _schema;

        bool _tmsScheme;
        std::vector<std::string> _tileURLs;
        std::vector<TileMask> _tileMasks;
        std::default_random_engine _randomGenerator;

        mutable std::recursive_mutex _mutex;
    };
    
}

#endif
