/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MEMORYCACHETILEDATASOURCE_H_
#define _CARTO_MEMORYCACHETILEDATASOURCE_H_

#include "datasources/CacheTileDataSource.h"

#include <stdext/timed_lru_cache.h>

namespace carto {

    /**
     * A tile data source that loads tiles from another tile data source
     * and caches them in memory. This cache is not persistent, tiles 
     * will be cleared once the application closes. Default cache capacity is 6MB.
     */
    class MemoryCacheTileDataSource : public CacheTileDataSource {
    public:
        /**
         * Constructs a MemoryCacheTileDataSource object from tile data source.
         * @param dataSource The datasource to be cached.
         */
        explicit MemoryCacheTileDataSource(const std::shared_ptr<TileDataSource>& dataSource);
        virtual ~MemoryCacheTileDataSource();
    
        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);
                
        virtual void clear();

        virtual std::size_t getCapacity() const;
        
        virtual void setCapacity(std::size_t capacityInBytes);
    
    protected:
        static const int DEFAULT_CAPACITY = 6 * 1024 * 1024;

        cache::timed_lru_cache<long long, std::shared_ptr<TileData> > _cache;
        mutable std::recursive_mutex _mutex;
    };
    
}

#endif
