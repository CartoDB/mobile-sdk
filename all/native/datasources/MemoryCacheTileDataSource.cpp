#include "MemoryCacheTileDataSource.h"
#include "core/BinaryData.h"
#include "core/MapTile.h"
#include "utils/Log.h"

#include <memory>

namespace carto {
    
    MemoryCacheTileDataSource::MemoryCacheTileDataSource(const std::shared_ptr<TileDataSource>& dataSource) :
        CacheTileDataSource(dataSource),
        _cache(DEFAULT_CAPACITY),
        _mutex()
    {
    }
    
    MemoryCacheTileDataSource::~MemoryCacheTileDataSource() {
    }
    
    std::shared_ptr<TileData> MemoryCacheTileDataSource::loadTile(const MapTile& mapTile) {
        std::unique_lock<std::recursive_mutex> lock(_mutex);
        
        Log::Infof("MemoryCacheTileDataSource::loadTile: Loading %s", mapTile.toString().c_str());
        
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
            Log::Infof("MemoryCacheTileDataSource::loadTile: Failed to load %s.", mapTile.toString().c_str());
        }
        
        return tileData;
    }
    
    void MemoryCacheTileDataSource::clear() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _cache.clear();
    }
    
    std::size_t MemoryCacheTileDataSource::getCapacity() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _cache.capacity();
    }
    
    void MemoryCacheTileDataSource::setCapacity(std::size_t capacityInBytes) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _cache.resize(capacityInBytes);
    }
        
}

