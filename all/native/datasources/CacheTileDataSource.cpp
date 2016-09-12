#include "CacheTileDataSource.h"
#include "core/MapTile.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <memory>

namespace carto {
    
    CacheTileDataSource::CacheTileDataSource(const std::shared_ptr<TileDataSource>& dataSource) :
        TileDataSource(), _dataSource(dataSource)
    {
        if (!dataSource) {
            throw NullArgumentException("Null dataSource");
        }

        _dataSourceListener = std::make_shared<DataSourceListener>(*this);
        _dataSource->registerOnChangeListener(_dataSourceListener);
    }
    
    CacheTileDataSource::~CacheTileDataSource() {
        _dataSource->unregisterOnChangeListener(_dataSourceListener);
        _dataSourceListener.reset();
    }

    int CacheTileDataSource::getMinZoom() const {
        return _dataSource->getMinZoom();
    }

    int CacheTileDataSource::getMaxZoom() const {
        return _dataSource->getMaxZoom();
    }

    void CacheTileDataSource::notifyTilesChanged(bool removeTiles) {
        clear();
        TileDataSource::notifyTilesChanged(removeTiles);
    }

    std::shared_ptr<TileDataSource> CacheTileDataSource::getDataSource() const {
        return _dataSource.get();
    }
    
    CacheTileDataSource::DataSourceListener::DataSourceListener(CacheTileDataSource& cacheDataSource) :
        _cacheDataSource(cacheDataSource)
    {
    }
    
    void CacheTileDataSource::DataSourceListener::onTilesChanged(bool removeTiles) {
        _cacheDataSource.notifyTilesChanged(removeTiles);
    }

}
