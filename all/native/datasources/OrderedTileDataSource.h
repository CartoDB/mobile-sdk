/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ORDEREDTILEDATASOURCE_H_
#define _CARTO_ORDEREDTILEDATASOURCE_H_

#include "datasources/TileDataSource.h"
#include "components/DirectorPtr.h"

namespace carto {
    
    /**
     * A tile data source that combines two data sources (usually offline and online).
     * All requests are made first to first data source. If not found the request will be made to the second data source.
     */
    class OrderedTileDataSource : public TileDataSource {
    public:
        /**
         * Constructs a combined tile data source object.
         * @param dataSource1 First data source that is used first if found
         * @param dataSource2 Second data source that is used only if dataSource1 tile is not found
         */
        OrderedTileDataSource(const std::shared_ptr<TileDataSource>& dataSource1, const std::shared_ptr<TileDataSource>& dataSource2);
        virtual ~OrderedTileDataSource();

        virtual int getMinZoom() const;
        virtual int getMaxZoom() const;

        virtual MapBounds getDataExtent() const;
        
        virtual std::shared_ptr<TileData> loadTile(const MapTile& tile);
        
    protected:
        class DataSourceListener : public TileDataSource::OnChangeListener {
        public:
            explicit DataSourceListener(OrderedTileDataSource& combinedDataSource);
            
            virtual void onTilesChanged(bool removeTiles);
            
        private:
            OrderedTileDataSource& _combinedDataSource;
        };
        
        const DirectorPtr<TileDataSource> _dataSource1;
        const DirectorPtr<TileDataSource> _dataSource2;
        
    private:
        std::shared_ptr<DataSourceListener> _dataSourceListener;
    };
    
}

#endif
