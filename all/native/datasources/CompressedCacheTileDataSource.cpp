#include "CompressedCacheTileDataSource.h"
#include "core/BinaryData.h"
#include "core/MapTile.h"
#include "utils/Log.h"

#include <memory>

namespace carto {
    
    CompressedCacheTileDataSource::CompressedCacheTileDataSource(const std::shared_ptr<TileDataSource>& dataSource) :
        CacheTileDataSource(dataSource),
        _cache(DEFAULT_CAPACITY),
        _mutex()
    {
    }
    
    CompressedCacheTileDataSource::~CompressedCacheTileDataSource() {
    }
    
    std::shared_ptr<TileData> CompressedCacheTileDataSource::loadTile(const MapTile& mapTile) {
        std::unique_lock<std::recursive_mutex> lock(_mutex);
        
        Log::Infof("CompressedCacheTileDataSource::loadTile: Loading %s", mapTile.toString().c_str());
        
        std::shared_ptr<TileData> tileData;
    if (_cache.read(mapTile.getTileId(), tileData)) {
            if (tileData->getMaxAge() != 0) {
                return tileData;
            }
            _cache.remove(mapTile.getTileId());
        }
        
        lock.unlock();
        tileData = _dataSource->loadTile(mapTile);
        lock.lock();

        if (tileData) {
            if (tileData->getMaxAge() != 0 && tileData->getData() && !tileData->isReplaceWithParent()) {
                _cache.put(mapTile.getTileId(), tileData, tileData->getData()->size() + 16);
            }
        } else {
            Log::Infof("CompressedCacheTileDataSource::loadTile: Failed to load %s.", mapTile.toString().c_str());
        }
        
        return tileData;
    }
    
    void CompressedCacheTileDataSource::clear() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _cache.clear();
    }
    
    unsigned int CompressedCacheTileDataSource::getCapacity() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _cache.capacity();
    }
    
    void CompressedCacheTileDataSource::setCapacity(unsigned int capacity) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _cache.resize(capacity);
    }
        
}

