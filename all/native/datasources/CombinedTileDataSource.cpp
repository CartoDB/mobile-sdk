#include "CombinedTileDataSource.h"
#include "core/MapTile.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <memory>

namespace carto {
    
    CombinedTileDataSource::CombinedTileDataSource(const std::shared_ptr<TileDataSource>& dataSource1, const std::shared_ptr<TileDataSource>& dataSource2, int zoomLevel) :
        TileDataSource(), _dataSource1(dataSource1), _dataSource2(dataSource2), _zoomLevel(zoomLevel)
    {
        if (!dataSource1) {
            throw NullArgumentException("Null dataSource1");
        }
        if (!dataSource2) {
            throw NullArgumentException("Null dataSource2");
        }

        _dataSourceListener = std::make_shared<DataSourceListener>(*this);
        _dataSource1->registerOnChangeListener(_dataSourceListener);
        _dataSource2->registerOnChangeListener(_dataSourceListener);
    }
    
    CombinedTileDataSource::~CombinedTileDataSource() {
        _dataSource2->unregisterOnChangeListener(_dataSourceListener);
        _dataSource1->unregisterOnChangeListener(_dataSourceListener);
        _dataSourceListener.reset();
    }

    int CombinedTileDataSource::getMinZoom() const {
        return _dataSource1 ? _dataSource1->getMinZoom() : 0;
    }

    int CombinedTileDataSource::getMaxZoom() const {
        return _dataSource2 ? _dataSource2->getMaxZoom() : 0;
    }
    
    std::shared_ptr<TileData> CombinedTileDataSource::loadTile(const MapTile& mapTile) {
        if (mapTile.getZoom() < _zoomLevel) {
            return _dataSource1->loadTile(mapTile);
        }
        return _dataSource2->loadTile(mapTile);
    }

    CombinedTileDataSource::DataSourceListener::DataSourceListener(CombinedTileDataSource& combinedDataSource) :
        _combinedDataSource(combinedDataSource)
    {
    }
    
    void CombinedTileDataSource::DataSourceListener::onTilesChanged(bool removeTiles) {
        _combinedDataSource.notifyTilesChanged(removeTiles);
    }
    
}
