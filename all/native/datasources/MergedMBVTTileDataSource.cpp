#include "MergedMBVTTileDataSource.h"
#include "core/BinaryData.h"
#include "core/MapTile.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <algorithm>

#include <stdext/miniz.h>

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
        std::shared_ptr<TileData> result1;
        std::shared_ptr<TileData> result2;
        if (zoom <= _dataSource1->getMaxZoom() && zoom >= _dataSource1->getMinZoom()) {
            result1 = _dataSource1->loadTile(mapTile);
        }
        if (zoom <= _dataSource2->getMaxZoom() && zoom >= _dataSource2->getMinZoom()) {
            result2 = _dataSource2->loadTile(mapTile);
        }

        if (result1 && result2) {
            // If either result contains 'replace with parent' then the only option is to pass this result on.
            // Otherwise we would need to do request the parent ourselves, do unpacking, scaling, clipping and packing.
            if (result1->isReplaceWithParent()) {
                return result1;
            }
            if (result2->isReplaceWithParent()) {
                return result2;
            }
            
            // We have data for both sources, we can merge them. Note that we may need to decompress the data first.
            std::shared_ptr<std::vector<unsigned char>> data1 = result1->getData()->getDataPtr();
            std::shared_ptr<std::vector<unsigned char>> data2 = result2->getData()->getDataPtr();

            std::vector<unsigned char> mergedData;
            mergedData.reserve(data1->size() + data2->size());

            std::vector<unsigned char> uncompressedData1;
            if (miniz::inflate_gzip(data1->data(), data1->size(), uncompressedData1)) {
                mergedData.insert(mergedData.end(), uncompressedData1.begin(), uncompressedData1.end());
            } else {
                mergedData.insert(mergedData.end(), data1->begin(), data1->end());
            }
            std::vector<unsigned char> uncompressedData2;
            if (miniz::inflate_gzip(data2->data(), data2->size(), uncompressedData2)) {
                mergedData.insert(mergedData.end(), uncompressedData2.begin(), uncompressedData2.end());
            } else {
                mergedData.insert(mergedData.end(), data2->begin(), data2->end());
            }

            auto mergedBinaryData = std::make_shared<BinaryData>(std::move(mergedData));
            return std::make_shared<TileData>(mergedBinaryData);
        }

        // Return either result that is not null.
        return result1 ? result1 : result2;
    }

    MergedMBVTTileDataSource::DataSourceListener::DataSourceListener(MergedMBVTTileDataSource& combinedDataSource) :
        _combinedDataSource(combinedDataSource)
    {
    }
    
    void MergedMBVTTileDataSource::DataSourceListener::onTilesChanged(bool removeTiles) {
        _combinedDataSource.notifyTilesChanged(removeTiles);
    }
    
}
