/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CACHETILEDATASOURCE_H_
#define _CARTO_CACHETILEDATASOURCE_H_

#include "datasources/TileDataSource.h"
#include "components/DirectorPtr.h"

namespace carto {
    
    /**
     * A tile data source that loads tiles from another tile data source and caches them.
     */
    class CacheTileDataSource : public TileDataSource {
    public:
        virtual ~CacheTileDataSource();
        
        virtual void notifyTilesChanged(bool removeTiles);
        
        /**
         * Clear the cache.
         */
        virtual void clear() = 0;
        
        /**
         * Returns the tile cache capacity.
         * @return The tile cache capacity in bytes.
         */
        virtual std::size_t getCapacity() const = 0;
        
        /**
         * Sets the cache capacity.
         * @param capacityInBytes The new tile cache capacity in bytes.
         */
        virtual void setCapacity(std::size_t capacityInBytes) = 0;

    protected:
        class DataSourceListener : public TileDataSource::OnChangeListener {
        public:
            DataSourceListener(CacheTileDataSource& cacheDataSource);
            
            virtual void onTilesChanged(bool removeTiles);
            
        private:
            CacheTileDataSource& _cacheDataSource;
        };
        
        CacheTileDataSource(const std::shared_ptr<TileDataSource>& dataSource);

        const DirectorPtr<TileDataSource> _dataSource;
        
    private:
        std::shared_ptr<DataSourceListener> _dataSourceListener;
    };
    
}

#endif
