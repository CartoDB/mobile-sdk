#include "CacheTileDataSource.h"
#include "core/MapTile.h"
#include "utils/Log.h"

#include <memory>

namespace carto {
    
    CacheTileDataSource::CacheTileDataSource(const std::shared_ptr<TileDataSource>& dataSource) :
        TileDataSource(dataSource->getMinZoom(), dataSource->getMaxZoom()), _dataSource(dataSource)
    {
        _dataSourceListener = std::make_shared<DataSourceListener>(*this);
        _dataSource->registerOnChangeListener(_dataSourceListener);
    }
    
    CacheTileDataSource::~CacheTileDataSource() {
        _dataSource->unregisterOnChangeListener(_dataSourceListener);
        _dataSourceListener.reset();
    }

    void CacheTileDataSource::notifyTilesChanged(bool removeTiles) {
        clear();
        TileDataSource::notifyTilesChanged(removeTiles);
    }
    
    CacheTileDataSource::DataSourceListener::DataSourceListener(CacheTileDataSource& cacheDataSource) :
        _cacheDataSource(cacheDataSource)
    {
    }
    
    void CacheTileDataSource::DataSourceListener::onTilesChanged(bool removeTiles) {
        _cacheDataSource.notifyTilesChanged(removeTiles);
    }

}
