#include "OrderedTileDataSource.h"
#include "core/MapTile.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <memory>
#include <algorithm>

namespace carto {
    
    OrderedTileDataSource::OrderedTileDataSource(const std::shared_ptr<TileDataSource>& dataSource1, const std::shared_ptr<TileDataSource>& dataSource2) :
        TileDataSource(),
        _dataSource1(dataSource1),
        _dataSource2(dataSource2)
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
    
    OrderedTileDataSource::~OrderedTileDataSource() {
        _dataSource2->unregisterOnChangeListener(_dataSourceListener);
        _dataSource1->unregisterOnChangeListener(_dataSourceListener);
        _dataSourceListener.reset();
    }

   int OrderedTileDataSource::getMinZoom() const {
        return std::min(_dataSource1->getMinZoom(), _dataSource2->getMinZoom());
    }

    int OrderedTileDataSource::getMaxZoom() const {
        return std::max(_dataSource1->getMinZoom(), _dataSource2->getMinZoom());
    }

    MapBounds OrderedTileDataSource::getDataExtent() const {
        MapBounds bounds = _dataSource1->getDataExtent();
        bounds.expandToContain(_dataSource2->getDataExtent());
        return bounds;
    }
    
    std::shared_ptr<TileData> OrderedTileDataSource::loadTile(const MapTile& mapTile) {
        int zoom = mapTile.getZoom();
        if (zoom <= _dataSource1->getMaxZoom() && zoom >= _dataSource1->getMinZoom()) {
            std::shared_ptr<TileData> result = _dataSource1->loadTile(mapTile);
            if (result && !result->isReplaceWithParent()) {
                return result;
            }
        }
        if (zoom <= _dataSource2->getMaxZoom() && zoom >= _dataSource2->getMinZoom()) {
            std::shared_ptr<TileData> result = _dataSource2->loadTile(mapTile);
            if (result && !result->isReplaceWithParent()) {
                return result;
            }
        }
        return std::shared_ptr<TileData>();
    }

    OrderedTileDataSource::DataSourceListener::DataSourceListener(OrderedTileDataSource& combinedDataSource) :
        _combinedDataSource(combinedDataSource)
    {
    }
    
    void OrderedTileDataSource::DataSourceListener::onTilesChanged(bool removeTiles) {
        _combinedDataSource.notifyTilesChanged(removeTiles);
    }
    
}
