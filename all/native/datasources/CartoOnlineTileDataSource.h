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
     * Tile server is currently used to 
     */
    class CartoOnlineTileDataSource : public TileDataSource {
    public:
        /**
         * Constructs a CartoOnlineTileDataSource object.
         * @param source Tile source id. For example "nutiteq.osm" for legacy tiles with old styles and "mapzen.osm" for latest MapZen tiles.
         */
        explicit CartoOnlineTileDataSource(const std::string& source);
        virtual ~CartoOnlineTileDataSource();

        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);
        
    protected:
        struct TileMask {
            bool inclusive = true;
            int maxZoom = -1;
            std::shared_ptr<PackageTileMask> tileMask;
            std::shared_ptr<BinaryData> tileData;
        };

        bool isMapZenSource() const;

        std::string buildTileURL(const std::string& baseURL, const MapTile& tile) const;

        bool loadTileURLs();

        std::shared_ptr<TileData> loadOnlineTile(const std::string& url, const MapTile& mapTile);

        static const int CARTO_MAX_ZOOM = 14;
        static const int MAPZEN_MAX_ZOOM = 17;
        static const int MAX_CACHED_TILES = 8;
        static const std::string TILE_SERVICE_URL;

        const std::string _source;
        mutable cache::timed_lru_cache<long long, std::shared_ptr<TileData> > _cache;
        HTTPClient _httpClient;

        bool _tmsScheme;
        std::vector<std::string> _tileURLs;
        std::vector<TileMask> _tileMasks;
        std::default_random_engine _randomGenerator;

        mutable std::recursive_mutex _mutex;
    };
    
}

#endif
