#include "PersistentCacheTileDataSource.h"
#include "core/BinaryData.h"
#include "datasources/TileDownloadListener.h"
#include "utils/Log.h"
#include "utils/TileUtils.h"

#include <memory>

#include <sqlite3pp.h>

namespace carto {
    
    PersistentCacheTileDataSource::PersistentCacheTileDataSource(const std::shared_ptr<TileDataSource>& dataSource, const std::string& databasePath) :
        CacheTileDataSource(dataSource),
        _database(),
        _cacheOnlyMode(false),
        _downloadThreadPool(std::make_shared<CancelableThreadPool>()),
        _cache(DEFAULT_CAPACITY),
        _mutex()
    {
        _downloadThreadPool->setPoolSize(1);
        openDatabase(databasePath);
    }
    
    PersistentCacheTileDataSource::~PersistentCacheTileDataSource() {
        stopAllDownloads();
        closeDatabase();
        _downloadThreadPool->deinit();
    }
    
    bool PersistentCacheTileDataSource::isCacheOnlyMode() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _cacheOnlyMode;
    }

    void PersistentCacheTileDataSource::setCacheOnlyMode(bool enabled) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _cacheOnlyMode = enabled;
    }

    void PersistentCacheTileDataSource::startDownloadArea(const MapBounds& mapBounds, int minZoom, int maxZoom, const std::shared_ptr<TileDownloadListener>& tileDownloadListener) {
        auto task = std::make_shared<DownloadTask>(std::static_pointer_cast<PersistentCacheTileDataSource>(shared_from_this()), mapBounds, minZoom, maxZoom, tileDownloadListener);
        _downloadThreadPool->execute(task, 0);
    }

    void PersistentCacheTileDataSource::stopAllDownloads() {
        _downloadThreadPool->cancelAll();
    }
    
    std::shared_ptr<TileData> PersistentCacheTileDataSource::loadTile(const MapTile& mapTile) {
        std::unique_lock<std::recursive_mutex> lock(_mutex);
        
        Log::Infof("PersistentCacheTileDataSource::loadTile: Loading %s", mapTile.toString().c_str());
        
        if (!_database) {
            Log::Error("PersistentCacheTileDataSource::loadTile: Could not connect to the database, loading tile without caching");
        }

        if (_cache.empty()) {
            loadTileInfo();
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
                long long tileId = mapTile.getTileId();
                std::size_t tileSize = tileData->getData()->size();
                _cache.put(mapTile.getTileId(), createTileId(tileId), tileSize + EXTRA_TILE_FOOTPRINT);
                if (_cache.exists(mapTile.getTileId())) { // make sure the tile was added
                    store(mapTile.getTileId(), tileData);
                }
            }
        } else {
            Log::Infof("PersistentCacheTileDataSource::loadTile: Failed to load %s", mapTile.toString().c_str());
        }
        
        return tileData;
    }

    bool PersistentCacheTileDataSource::isOpen() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return (bool) _database;
    }

    void PersistentCacheTileDataSource::close() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        closeDatabase();
    }
        
    void PersistentCacheTileDataSource::clear() {
        try {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _cache.clear(); // forces all elements to be removed, but can be slow
        }
        catch (const std::exception& ex) {
            Log::Errorf("PersistentCacheTileDataSource::clear: Failed to clear cache: %s", ex.what());
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
        }
        catch (const std::exception& ex) {
            Log::Errorf("PersistentCacheTileDataSource::openDatabase: Failed to connect to database: %s", ex.what());
            _database.reset();
            return;
        }
        
        try {
            sqlite3pp::command command1(*_database, "PRAGMA page_size=4096");
            command1.execute();
            command1.finish();
            
            try {
                sqlite3pp::query query1(*_database, "SELECT name FROM sqlite_master WHERE type='table' AND name='persistent_cache'");
                for (auto it1 = query1.begin(); it1 != query1.end(); ++it1) {
                    sqlite3pp::query query2(*_database, "SELECT expirationTime FROM persistent_cache");
                    for (auto it2 = query2.begin(); it2 != query2.end(); ++it2);
                    query2.finish();
                }
                query1.finish();
            }
            catch (const std::exception&) {
                Log::Info("PersistentCacheTileDataSource::openDatabase: Reinitializing database");
                sqlite3pp::command command(*_database, "DROP TABLE IF EXISTS persistent_cache");
                command.execute();
                command.finish();
            }

            sqlite3pp::command command3(*_database, "CREATE TABLE IF NOT EXISTS persistent_cache(tileId INTEGER NOT NULL PRIMARY KEY, compressed BLOB, time INTEGER, expirationTime INTEGER)");
            command3.execute();
            command3.finish();
        }
        catch (const std::exception& ex) {
            Log::Errorf("PersistentCacheTileDataSource::openDatabase: Failed to initialize database: %s", ex.what());
            _database.reset();
            return;
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
        }
        catch (const std::exception& ex) {
            Log::Errorf("PersistentCacheTileDataSource::closeDatabase: Failed to close database: %s", ex.what());
            _database.reset();
        }

        _cache.clear(); // NOTE: as the database is closed at this point, elements are not removed
    }
    
    void PersistentCacheTileDataSource::loadTileInfo() {
        struct TileInfo {
            std::uint64_t tileId;
            std::size_t tileSize;
            std::uint64_t time;
        };

        if (!_database) {
            return;
        }

        try {
            // Get tile ids and sizes ordered by the timestamp from the database
            std::vector<TileInfo> tileInfos;
            tileInfos.reserve(_cache.capacity() / (EXTRA_TILE_FOOTPRINT + 1));
            sqlite3pp::query query(*_database, "SELECT tileId, LENGTH(compressed), time FROM persistent_cache");
            for (auto it = query.begin(); it != query.end(); ++it) {
                TileInfo tileInfo;
                tileInfo.tileId = (*it).get<std::uint64_t>(0);
                tileInfo.tileSize = static_cast<std::size_t>((*it).get<std::uint64_t>(1));
                tileInfo.time = (*it).get<std::uint64_t>(2);
                tileInfos.push_back(tileInfo);
            }
            query.finish();

            // Sort the tiles
            std::sort(tileInfos.begin(), tileInfos.end(), [](const TileInfo& tileInfo1, const TileInfo& tileInfo2) {
                return tileInfo1.time < tileInfo2.time;
            });

            // Now store the queried items in cache. This may result in eviction of some of the items.
            for (const TileInfo& tileInfo : tileInfos) {
                _cache.put(tileInfo.tileId, createTileId(tileInfo.tileId), tileInfo.tileSize + EXTRA_TILE_FOOTPRINT);
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("PersistentCacheTileDataSource::loadTileInfo: Failed to query tile set from the database: %s", ex.what());
        }
    }
    
    std::shared_ptr<TileData> PersistentCacheTileDataSource::get(long long tileId) {
        if (!_database) {
            return std::shared_ptr<TileData>();
        }
    
        try {
            // Get the tile from the database
            sqlite3pp::query query(*_database, "SELECT compressed, expirationTime FROM persistent_cache WHERE tileId=:tileId");
            query.bind(":tileId", static_cast<std::uint64_t>(tileId));
            auto qit = query.begin();
            if (qit == query.end()) {
                // No data exists for this tile in the database
                Log::Error("PersistentCacheTileDataSource::get: Inconsistency, tile data does not exist in the database");
                return std::shared_ptr<TileData>();
            }
            
            // Construct TileData from the blob returned from the database
            std::size_t dataSize = (*qit).column_bytes(0);
            const unsigned char* dataPtr = static_cast<const unsigned char*>((*qit).get<const void*>(0));
            long long expirationTime = (*qit).get<std::uint64_t>(1);
            auto data = std::make_shared<BinaryData>(dataPtr, dataSize);
            query.finish();
            
            auto tileData = std::make_shared<TileData>(data);
            if (expirationTime != 0) {
                long long maxAge = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::time_point(std::chrono::milliseconds(expirationTime)) - std::chrono::system_clock::now()).count();
                tileData->setMaxAge(maxAge > 0 ? maxAge : 0);
            }
            return tileData;
        }
        catch (const std::exception& ex) {
            Log::Errorf("PersistentCacheTileDataSource::get: Failed to query tile data from the database: %s", ex.what());
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
            command.bind(":tileId", static_cast<std::uint64_t>(tileId));
            command.bind(":compressed", tileData->getData()->data(), static_cast<unsigned int>(tileData->getData()->size()));
            command.bind(":time", static_cast<std::uint64_t>(time));
            command.bind(":expirationTime", static_cast<std::uint64_t>(expirationTime));
            command.execute();
            command.finish();
        }
        catch (const std::exception& ex) {
            Log::Errorf("PersistentCacheTileDataSource::store: Failed to store tile data in the database: %s", ex.what());
        }
    }

    void PersistentCacheTileDataSource::remove(long long tileId) {
        if (!_database) {
            return;
        }
        
        try {
            sqlite3pp::command command(*_database, "DELETE FROM persistent_cache WHERE tileId=:tileId");
            command.bind(":tileId", static_cast<std::uint64_t>(tileId));
            command.execute();
            command.finish();
        }
        catch (const std::exception& ex) {
            Log::Errorf("PersistentCacheTileDataSource::remove: Failed to remove tile from the database: %s", ex.what());
        }
    }
    
    std::shared_ptr<long long> PersistentCacheTileDataSource::createTileId(long long tileId) {
        std::weak_ptr<PersistentCacheTileDataSource> cacheWeak(std::static_pointer_cast<PersistentCacheTileDataSource>(shared_from_this()));
        auto tileIdDeleter = [cacheWeak](long long* tileIdPtr) {
            std::unique_ptr<long long> tileId(tileIdPtr);
            if (auto cache = cacheWeak.lock()) {
                std::lock_guard<std::recursive_mutex> lock(cache->_mutex); // probably not needed, as this gets called from already locked state
                cache->remove(*tileId);
            }
        };
        return std::shared_ptr<long long>(new long long(tileId), tileIdDeleter);
    }

    PersistentCacheTileDataSource::DownloadTask::DownloadTask(const std::shared_ptr<PersistentCacheTileDataSource>& dataSource, const MapBounds& mapBounds, int minZoom, int maxZoom, const std::shared_ptr<TileDownloadListener>& listener) :
        _dataSource(dataSource),
        _mapBounds(mapBounds),
        _minZoom(minZoom),
        _maxZoom(maxZoom),
        _downloadListener(listener)
    {
    }
    
    void PersistentCacheTileDataSource::DownloadTask::run() {
        std::shared_ptr<Projection> projection;
        int minZoom = _minZoom;
        int maxZoom = _maxZoom;
        if (auto dataSource = _dataSource.lock()) {
            if (!dataSource->isOpen()) {
                Log::Warn("PersistentCacheTileDataSource::DownloadTask: Database is not open, skipping download");
                return;
            }
            projection = dataSource->getProjection();
            minZoom = std::max(minZoom, dataSource->getMinZoom());
            maxZoom = std::min(maxZoom, dataSource->getMaxZoom());
        } else {
            return;
        }

        std::uint64_t tileCount = 0;
        for (int zoom = minZoom; zoom <= maxZoom; zoom++) {
            MapTile mapTile1 = TileUtils::CalculateMapTile(_mapBounds.getMin(), zoom, projection);
            MapTile mapTile2 = TileUtils::CalculateMapTile(_mapBounds.getMax(), zoom, projection);
            std::uint64_t dx = std::abs(mapTile1.getX() - mapTile2.getX()) + 1;
            std::uint64_t dy = std::abs(mapTile1.getY() - mapTile2.getY()) + 1;
            tileCount += dx * dy;
        }

        Log::Infof("PersistentCacheTileDataSource::DownloadTask: Starting to download %d tiles", static_cast<int>(tileCount));

        if (_downloadListener) {
            _downloadListener->onDownloadStarting(static_cast<int>(tileCount));
        }

        std::uint64_t tileIndex = 0;
        for (int zoom = minZoom; zoom <= maxZoom; zoom++) {
            MapTile mapTile1 = TileUtils::CalculateMapTile(_mapBounds.getMin(), zoom, projection);
            MapTile mapTile2 = TileUtils::CalculateMapTile(_mapBounds.getMax(), zoom, projection);
            for (int y = std::min(mapTile1.getY(), mapTile2.getY()); y <= std::max(mapTile1.getY(), mapTile2.getY()); y++) {
                if (isCanceled()) {
                    break;
                }

                for (int x = std::min(mapTile1.getX(), mapTile2.getX()); x <= std::max(mapTile1.getX(), mapTile2.getX()); x++) {
                    if (isCanceled()) {
                        break;
                    }

                    if (_downloadListener) {
                        _downloadListener->onDownloadProgress(static_cast<float>(100.0 * tileIndex / tileCount));
                    }

                    MapTile mapTile(x, y, zoom, 0);
                    std::shared_ptr<TileData> tileData;
                    if (auto dataSource = _dataSource.lock()) {
                        tileData = dataSource->loadTile(mapTile.getFlipped());
                    } else {
                        return;
                    }
                    tileIndex++;

                    if (!tileData && _downloadListener) {
                        _downloadListener->onDownloadFailed(mapTile);
                    }
                }
            }
        }

        if (tileIndex == tileCount && _downloadListener) {
            _downloadListener->onDownloadProgress(100.0f);
            _downloadListener->onDownloadCompleted();
        }

        Log::Info("PersistentCacheTileDataSource::DownloadTask: Finished downloading");
    }

    const unsigned int PersistentCacheTileDataSource::DEFAULT_CAPACITY = 50 * 1024 * 1024;
    const unsigned int PersistentCacheTileDataSource::EXTRA_TILE_FOOTPRINT = 1024;

}
