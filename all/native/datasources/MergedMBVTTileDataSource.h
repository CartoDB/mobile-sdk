/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MergedMBVTTileDataSource_H_
#define _CARTO_MergedMBVTTileDataSource_H_

#include "datasources/TileDataSource.h"
#include "components/DirectorPtr.h"

namespace carto {
    
    /**
     * A tile data source that combines two MBVT/protobuf  data sources into one */
    class MergedMBVTTileDataSource : public TileDataSource {
    public:
        /**
         * Constructs a combined tile data source object.
         * @param dataSource1 First data source to be combined
         * @param dataSource2 Second ddata source to be combined
         */
        MergedMBVTTileDataSource(const std::shared_ptr<TileDataSource>& dataSource1, const std::shared_ptr<TileDataSource>& dataSource2);
        virtual ~MergedMBVTTileDataSource();

        virtual int getMinZoom() const;
        virtual int getMaxZoom() const;

        virtual MapBounds getDataExtent() const;
        
        virtual std::shared_ptr<TileData> loadTile(const MapTile& tile);
        
    protected:
        class DataSourceListener : public TileDataSource::OnChangeListener {
        public:
            explicit DataSourceListener(MergedMBVTTileDataSource& combinedDataSource);
            
            virtual void onTilesChanged(bool removeTiles);
            
        private:
            MergedMBVTTileDataSource& _combinedDataSource;
        };
        
        const DirectorPtr<TileDataSource> _dataSource1;
        const DirectorPtr<TileDataSource> _dataSource2;
        
    private:
        std::shared_ptr<DataSourceListener> _dataSourceListener;
    };
    
}

#endif
