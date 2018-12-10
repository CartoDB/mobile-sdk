#include "MergedMBVTTileDataSource.h"
#include "core/BinaryData.h"
#include "core/MapTile.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <memory>
#include <algorithm>

namespace carto {
    
    MergedMBVTTileDataSource::MergedMBVTTileDataSource(const std::shared_ptr<TileDataSource>& dataSource1, const std::shared_ptr<TileDataSource>& dataSource2) :
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
    
    MergedMBVTTileDataSource::~MergedMBVTTileDataSource() {
        _dataSource2->unregisterOnChangeListener(_dataSourceListener);
        _dataSource1->unregisterOnChangeListener(_dataSourceListener);
        _dataSourceListener.reset();
    }

   int MergedMBVTTileDataSource::getMinZoom() const {
        return std::min(_dataSource1->getMinZoom(), _dataSource2->getMinZoom());
    }

    int MergedMBVTTileDataSource::getMaxZoom() const {
        return std::max(_dataSource1->getMaxZoom(), _dataSource2->getMaxZoom());
    }

    MapBounds MergedMBVTTileDataSource::getDataExtent() const {
        MapBounds bounds = _dataSource1->getDataExtent();
        bounds.expandToContain(_dataSource2->getDataExtent());
        return bounds;
    }
    
    std::shared_ptr<TileData> MergedMBVTTileDataSource::loadTile(const MapTile& mapTile) {
        int zoom = mapTile.getZoom();
        std::shared_ptr<TileData> result1 = NULL;
        std::shared_ptr<TileData> result2 = NULL;
        if (zoom <= _dataSource1->getMaxZoom() && zoom >= _dataSource1->getMinZoom()) {
            result1 = _dataSource1->loadTile(mapTile);
        }
        if (zoom <= _dataSource2->getMaxZoom() && zoom >= _dataSource2->getMinZoom()) {
            result2 = _dataSource2->loadTile(mapTile);
        }
        if (result1 && result2 
            && !result1->isReplaceWithParent() && !result1->isReplaceWithParent()) {
            
            // we have data for both sources, we can merge them
            // std::shared_ptr<BinaryData> binaryData1 = result1.getData();
            // std::shared_ptr<BinaryData> binaryData2 = result2.getData();

            std::shared_ptr<std::vector<unsigned char> > data1 = result1->getData()->getDataPtr();
            std::shared_ptr<std::vector<unsigned char> > data2 = result2->getData()->getDataPtr();

            std::vector<unsigned char> mergedData(*data1);
            mergedData.insert(mergedData.end(), data2->begin(), data2->end());
            auto mergedBinaryData = std::make_shared<BinaryData>(std::move(mergedData));

            return std::make_shared<TileData>(mergedBinaryData);
        } else if (result1 && !result1->isReplaceWithParent()) {
            return result1;
        } else if (result2 && !result2->isReplaceWithParent()) {
            return result2;
        }
        return std::shared_ptr<TileData>();
    }

    MergedMBVTTileDataSource::DataSourceListener::DataSourceListener(MergedMBVTTileDataSource& combinedDataSource) :
        _combinedDataSource(combinedDataSource)
    {
    }
    
    void MergedMBVTTileDataSource::DataSourceListener::onTilesChanged(bool removeTiles) {
        _combinedDataSource.notifyTilesChanged(removeTiles);
    }
    
}
