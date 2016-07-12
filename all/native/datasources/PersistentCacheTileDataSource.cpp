#include "PersistentCacheTileDataSource.h"
#include "core/BinaryData.h"
#include "core/MapTile.h"
#include "utils/Log.h"

#include <memory>

#include <sqlite3pp.h>

namespace carto {
    
    PersistentCacheTileDataSource::PersistentCacheTileDataSource(const std::shared_ptr<TileDataSource>& dataSource, const std::string& databasePath) :
        CacheTileDataSource(dataSource),
        _database(),
        _cacheOnlyMode(false),
        _cache(DEFAULT_CAPACITY),
        _mutex()
    {
        openDatabase(databasePath);
    }
    
    PersistentCacheTileDataSource::~PersistentCacheTileDataSource() {
        closeDatabase();
    }
    
    bool PersistentCacheTileDataSource::isCacheOnlyMode() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _cacheOnlyMode;
    }

    void PersistentCacheTileDataSource::setCacheOnlyMode(bool enabled) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _cacheOnlyMode = enabled;
    }
    
    std::shared_ptr<TileData> PersistentCacheTileDataSource::loadTile(const MapTile& mapTile) {
        std::unique_lock<std::recursive_mutex> lock(_mutex);
        
        Log::Infof("PersistentCacheTileDataSource::loadTile: Loading %s", mapTile.toString().c_str());
        
        if (!_database) {
            Log::Error("PersistentCacheTileDataSource::loadTile: Could not connect to the database, loading tile without caching");
        }
        
        std::shared_ptr<TileData> tileData;

        std::shared_ptr<long long> tileIdPtr;
        if (_cache.read(mapTile.getTileId(), tileIdPtr)) {
            tileData = get(mapTile.getTileId());
            if (tileData) {
                if (tileData->getMaxAge() != 0) {
                    return tileData;
                }
            }
            _cache.remove(mapTile.getTileId());
        }
        
        if (!_cacheOnlyMode) {
            lock.unlock();
            tileData = _dataSource->loadTile(mapTile);
            lock.lock();
        }
    
        if (tileData) {
            if (tileData->getMaxAge() != 0 && !tileData->isReplaceWithParent() && tileData->getData()) {
                _cache.put(mapTile.getTileId(), createTileId(mapTile.getTileId()), tileData->getData()->size());
                if (_cache.exists(mapTile.getTileId())) { // make sure the tile was added
                    store(mapTile.getTileId(), tileData);
                }
            }
        } else {
            Log::Infof("PersistentCacheTileDataSource::loadTile: Failed to load %s", mapTile.toString().c_str());
        }
        
        return tileData;
    }

    void PersistentCacheTileDataSource::close() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        closeDatabase();
    }
        
    void PersistentCacheTileDataSource::clear() {
        try {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _cache.clear(); // forces all elements to be removed, but can be slow
        } catch (const std::exception& e) {
            Log::Errorf("PersistentCacheTileDataSource::clear: Failed to clear cache: %s", e.what());
        }
    }
    
    std::size_t PersistentCacheTileDataSource::getCapacity() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _cache.capacity();
    }
    
    void PersistentCacheTileDataSource::setCapacity(std::size_t capacityInBytes) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _cache.resize(capacityInBytes);
    }
    
    void PersistentCacheTileDataSource::openDatabase(const std::string& databasePath) {
        try {
            _database.reset(new sqlite3pp::database(databasePath.c_str()));
        } catch (const std::exception& e) {
            Log::Errorf("PersistentCacheTileDataSource::openDatabase: Failed to connect to database: %s", e.what());
            _database.reset();
            return;
        }
        
        try {
            sqlite3pp::command command1(*_database, "PRAGMA page_size=4096");
            command1.execute();
            command1.finish();
            
            sqlite3pp::command command2(*_database, "PRAGMA cache_size=1"); // use minimal amount of cache
            command2.execute();
            command2.finish();

            try {
                sqlite3pp::query query1(*_database, "SELECT name FROM sqlite_master WHERE type='table' AND name='persistent_cache'");
                for (auto it1 = query1.begin(); it1 != query1.end(); ++it1) {
                    sqlite3pp::query query2(*_database, "SELECT expirationTime FROM persistent_cache");
                    for (auto it2 = query2.begin(); it2 != query2.end(); ++it2);
                    query2.finish();
                }
                query1.finish();
            } catch (const std::exception&) {
                Log::Info("PersistentCacheTileDataSource::openDatabase: Reinitializing database");
                sqlite3pp::command command(*_database, "DROP TABLE IF EXISTS persistent_cache");
                command.execute();
                command.finish();
            }

            sqlite3pp::command command3(*_database, "CREATE TABLE IF NOT EXISTS persistent_cache(tileId INTEGER NOT NULL PRIMARY KEY, compressed BLOB, time INTEGER, expirationTime INTEGER)");
            command3.execute();
            command3.finish();
        } catch (const std::exception& e) {
            Log::Errorf("PersistentCacheTileDataSource::openDatabase: Failed to initialize database: %s", e.what());
            _database.reset();
            return;
        }
        
        // Get tile ids and sizes ordered by the timestamp from the database
        try {
            sqlite3pp::query query(*_database, "SELECT tileId, LENGTH(compressed) FROM persistent_cache ORDER BY time ASC");
            for (auto it = query.begin(); it != query.end(); ++it) {
                long long tileId = (*it).get<uint64_t>(0);
                int tileSize = (*it).get<int>(1);
                if (_cache.capacity() < _cache.size() + tileSize) {
                    _cache.resize(_cache.size() + tileSize); // if cache size is not configured yet, just increase the capacity until everything can be loaded
                }
                _cache.put(tileId, createTileId(tileId), tileSize);
            }
            query.finish();
        } catch (const std::exception& e) {
            Log::Errorf("PersistentCacheTileDataSource::openDatabase: Failed to query tile set from the database: %s", e.what());
        }
    }
    
    void PersistentCacheTileDataSource::closeDatabase() {
        if (!_database) {
            return;
        }

        try {
            if (_database->disconnect() != SQLITE_OK) {
                Log::Error("PersistentCacheTileDataSource::closeDatabase: Failed to close database");
            }
            _database.reset();
        } catch (const std::exception& e) {
            Log::Errorf("PersistentCacheTileDataSource::closeDatabase: Failed to close database: %s", e.what());
            _database.reset();
        }

        _cache.clear(); // NOTE: as the database is closed at this point, elements are not removed
    }
    
    std::shared_ptr<TileData> PersistentCacheTileDataSource::get(long long tileId) {
        if (!_database) {
            return std::shared_ptr<TileData>();
        }
    
        try {
            // Get the tile from the database
            sqlite3pp::query query(*_database, "SELECT compressed, LENGTH(compressed), expirationTime FROM persistent_cache WHERE tileId=:tileId");
            query.bind(":tileId", static_cast<uint64_t>(tileId));
            auto qit = query.begin();
            if (qit == query.end()) {
                // No data exists for this tile in the database
                Log::Error("PersistentCacheTileDataSource::get: Inconsistency, tile data does not exist in the database");
                return std::shared_ptr<TileData>();
            }
            
            // Construct TileData from the blob returned from the database
            const unsigned char* dataPtr = static_cast<const unsigned char*>((*qit).get<const void*>(0));
            std::size_t dataSize = (*qit).get<int>(1);
            long long expirationTime = (*qit).get<std::uint64_t>(2);
            auto data = std::make_shared<BinaryData>(dataPtr, dataSize);
            query.finish();
            
            auto tileData = std::make_shared<TileData>(data);
            if (expirationTime != 0) {
                long long maxAge = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::time_point(std::chrono::milliseconds(expirationTime)) - std::chrono::system_clock::now()).count();
                tileData->setMaxAge(maxAge > 0 ? maxAge : 0);
            }
            return tileData;
        } catch (const std::exception& e) {
            Log::Errorf("PersistentCacheTileDataSource::get: Failed to query tile data from the database: %s", e.what());
            return std::shared_ptr<TileData>();
        }
    }
    
    void PersistentCacheTileDataSource::store(long long tileId, const std::shared_ptr<TileData>& tileData) {
        if (!_database) {
            return;
        }
        
        long long time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        long long expirationTime = 0;
        if (tileData->getMaxAge() >= 0) {
            expirationTime = std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::system_clock::now() + std::chrono::milliseconds(tileData->getMaxAge())).time_since_epoch()).count();
        }

        // Add tile to the database
        try {
            sqlite3pp::command command(*_database, "INSERT OR REPLACE INTO persistent_cache(tileId, compressed, time, expirationTime) VALUES (:tileId, :compressed, :time, :expirationTime)");
            command.bind(":tileId", static_cast<uint64_t>(tileId));
            command.bind(":compressed", tileData->getData()->data(), static_cast<unsigned int>(tileData->getData()->size()));
            command.bind(":time", static_cast<uint64_t>(time));
            command.bind(":expirationTime", static_cast<uint64_t>(expirationTime));
            command.execute();
            command.finish();
        } catch (const std::exception& e) {
            Log::Errorf("PersistentCacheTileDataSource::store: Failed to store tile data in the database: %s", e.what());
        }
    }

    void PersistentCacheTileDataSource::remove(long long tileId) {
        if (!_database) {
            return;
        }
        
        try {
            sqlite3pp::command command(*_database, "DELETE FROM persistent_cache WHERE tileId=:tileId");
            command.bind(":tileId", static_cast<uint64_t>(tileId));
            command.execute();
            command.finish();
        } catch (const std::exception& e) {
            Log::Errorf("PersistentCacheTileDataSource::remove: Failed to remove tile from the database: %s", e.what());
        }
    }
    
    std::shared_ptr<long long> PersistentCacheTileDataSource::createTileId(long long tileId) {
        return std::shared_ptr<long long>(new long long(tileId), [=](long long* tileId) {
            std::lock_guard<std::recursive_mutex> lock(_mutex); // probably not needed, as this gets called from already locked state
            remove(*tileId);
            delete tileId;
        });
    }
    
}
