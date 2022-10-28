#ifdef _CARTO_OFFLINE_SUPPORT

#include "MBTilesTileDataSource.h"
#include "core/BinaryData.h"
#include "core/MapTile.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <sqlite3pp.h>

namespace carto {

    MBTilesTileDataSource::MBTilesTileDataSource(const std::string& path) :
        TileDataSource(),
        _scheme(MBTilesScheme::MBTILES_SCHEME_TMS),
        _database(OpenDatabase(path)),
        _cachedMinZoom(),
        _cachedMaxZoom(),
        _cachedDataExtent(),
        _mutex()
    {
    }

    MBTilesTileDataSource::MBTilesTileDataSource(int minZoom, int maxZoom, const std::string& path) :
        TileDataSource(minZoom, maxZoom),
        _scheme(MBTilesScheme::MBTILES_SCHEME_TMS),
        _database(OpenDatabase(path)),
        _cachedMinZoom(minZoom),
        _cachedMaxZoom(maxZoom),
        _cachedDataExtent(),
        _mutex()
    {
    }
    
    MBTilesTileDataSource::MBTilesTileDataSource(int minZoom, int maxZoom, const std::string& path, MBTilesScheme::MBTilesScheme scheme) :
        TileDataSource(minZoom, maxZoom),
        _scheme(scheme),
        _database(OpenDatabase(path)),
        _cachedMinZoom(minZoom),
        _cachedMaxZoom(maxZoom),
        _cachedDataExtent(),
        _mutex()
    {
    }
        
    MBTilesTileDataSource::~MBTilesTileDataSource() {
    }
    
    std::map<std::string, std::string> MBTilesTileDataSource::getMetaData() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (!_database) {
            Log::Error("MBTilesTileDataSource::getMetaData: Not connected to the database");
            return std::map<std::string, std::string>();
        }
        
        try {
            // Make the query and check for database error
            std::map<std::string, std::string> metaData;
            sqlite3pp::query query(*_database, "SELECT name, value FROM metadata");
            for (auto it = query.begin(); it != query.end(); it++) {
                metaData[it->get<const char*>(0)] = it->get<const char*>(1);
            }
            query.finish();
            return metaData;
        }
        catch (const std::exception& ex) {
            Log::Errorf("MBTilesTileDataSource::getMetaData: Failed to query metadata from the database: %s", ex.what());
            return std::map<std::string, std::string>();
        }
    }

    int MBTilesTileDataSource::getMinZoom() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        // If value is not cached, load from database
        if (!_cachedMinZoom) {
            if (!_database) {
                Log::Error("MBTilesTileDataSource::getMinZoom: Not connected to the database");
                return 0;
            }

            int minZoom = 0, maxZoom = -1;
            loadZoomLevels(minZoom, maxZoom);
            _cachedMinZoom = minZoom;
            _cachedMaxZoom = maxZoom;
        }
        return *_cachedMinZoom;
    }
    
    int MBTilesTileDataSource::getMaxZoom() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        // If value is not cached, load from database
        if (!_cachedMaxZoom) {
            if (!_database) {
                Log::Error("MBTilesTileDataSource::getMaxZoom: Not connected to the database");
                return -1;
            }

            int minZoom = 0, maxZoom = -1;
            loadZoomLevels(minZoom, maxZoom);
            _cachedMinZoom = minZoom;
            _cachedMaxZoom = maxZoom;
        }
        return *_cachedMaxZoom;
    }
    
    MapBounds MBTilesTileDataSource::getDataExtent() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        // If value is not cached, load from database
        if (!_cachedDataExtent) {
            if (!_database) {
                Log::Error("MBTilesTileDataSource::getDataExtent: Not connected to the database");
                return MapBounds();
            }

            MapBounds mapBounds;
            loadDataExtent(mapBounds);
            _cachedDataExtent = mapBounds;
        }
        return *_cachedDataExtent;
    }

    std::shared_ptr<TileData> MBTilesTileDataSource::loadTile(const MapTile& mapTile) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        Log::Infof("MBTilesTileDataSource::loadTile: Loading %s", mapTile.toString().c_str());
        if (!_database) {
            Log::Errorf("MBTilesTileDataSource::loadTile: Failed to load %s: Couldn't connect to the database", mapTile.toString().c_str());
            return std::shared_ptr<TileData>();
        }

        if (getMaxOverzoomLevel() >= 0 && mapTile.getZoom() > getMaxZoomWithOverzoom()) {
            // we explicitly return an empty tile to not draw overzoom
            auto tileData = std::make_shared<TileData>(std::shared_ptr<BinaryData>());
            tileData->setIsOverZoom(true);
            return  tileData;
        }
        try {
            // Make the query and check for database error
            sqlite3pp::query query(*_database, "SELECT tile_data FROM tiles WHERE zoom_level=:zoom AND tile_column=:x AND tile_row=:y");
            query.bind(":zoom", mapTile.getZoom());
            query.bind(":x", mapTile.getX());
            query.bind(":y", _scheme == MBTilesScheme::MBTILES_SCHEME_XYZ ? mapTile.getY() : (1 << (mapTile.getZoom())) - 1 - mapTile.getY());
            
            auto it = query.begin();
            if (it == query.end()) {
                std::shared_ptr<TileData> tileData = std::make_shared<TileData>(std::shared_ptr<BinaryData>());
                if (mapTile.getZoom() > getMinZoom()) {
                    Log::Infof("MBTilesTileDataSource::loadTile: Tile data doesn't exist in the database, redirecting to parent");
                    tileData->setReplaceWithParent(true);
                } else {
                    Log::Infof("MBTilesTileDataSource::loadTile: Tile data doesn't exist in the database");
                    return std::shared_ptr<TileData>();
                }
                return tileData;
            }
            
            std::size_t dataSize = (*it).column_bytes(0);
            const unsigned char* dataPtr = static_cast<const unsigned char*>((*it).get<const void*>(0));
            auto data = std::make_shared<BinaryData>(dataPtr, dataSize);
            query.finish();
    
            return std::make_shared<TileData>(data);
        }
        catch (const std::exception& ex) {
            Log::Errorf("MBTilesTileDataSource::loadTile: Failed to query tile data from the database: %s", ex.what());
            return std::shared_ptr<TileData>();
        }
    }

    std::unique_ptr<sqlite3pp::database> MBTilesTileDataSource::OpenDatabase(const std::string& path) {
        auto database = std::make_unique<sqlite3pp::database>();
        if (database->connect_v2(path.c_str(), SQLITE_OPEN_READONLY) != SQLITE_OK) {
            throw FileException("Failed to open database file", path);
        }
        database->execute("PRAGMA temp_store=MEMORY");
        return database;
    }

    bool MBTilesTileDataSource::loadZoomLevels(int& minZoom, int& maxZoom) const {
        // First try to use metadata table for min/maxzoom values
        bool foundMinZoom = false, foundMaxZoom = false;
        try {
            sqlite3pp::query query(*_database, "SELECT name, value FROM metadata WHERE name IN ('minzoom', 'maxzoom')");
            for (auto it = query.begin(); it != query.end(); it++) {
                std::string name = (*it).get<const char*>(0);
                std::string strValue = (*it).get<const char*>(1);
                int numValue = (strValue.substr(0, 3) == "inf" ? Const::MAX_SUPPORTED_ZOOM_LEVEL : boost::lexical_cast<int>(strValue));
                if (name == "minzoom") {
                    minZoom = numValue;
                    foundMinZoom = true;
                } else if (name == "maxzoom") {
                    maxZoom = numValue;
                    foundMaxZoom = true;
                }
            }
            query.finish();
        }
        catch (const std::exception& ex) {
            Log::Errorf("MBTilesTileDataSource: Exception while reading zoom level metadata: %s", ex.what());
        }

        // If either was not found, do table scan
        if (!foundMinZoom || !foundMaxZoom) {
            try {
                sqlite3pp::query query(*_database, "SELECT MIN(zoom_level), MAX(zoom_level) FROM tiles");
                for (auto it = query.begin(); it != query.end(); it++) {
                    if (!foundMinZoom) {
                        minZoom = it->get<int>(0);
                    }
                    if (!foundMaxZoom) {
                        maxZoom = it->get<int>(1);
                    }
                }
                query.finish();
            }
            catch (const std::exception& ex) {
                Log::Errorf("MBTilesTileDataSource: Failed to read min/max zoom info: %s", ex.what());
                return false;
            }
        }
        return true;
    }
    
    bool MBTilesTileDataSource::loadDataExtent(MapBounds& mapBounds) const {
        // As a first step, try to use metadata
        bool foundMapBounds = false;
        try {
            sqlite3pp::query query(*_database, "SELECT value FROM metadata WHERE name='bounds'");
            for (auto it = query.begin(); it != query.end(); it++) {
                std::string bounds = (*it).get<const char*>(0);
                std::vector<std::string> coordinates;
                boost::split(coordinates, bounds, boost::is_any_of(","));
                if (coordinates.size() == 4) {
                    double x0 = boost::lexical_cast<double>(boost::trim_copy(coordinates[0]));
                    double y0 = boost::lexical_cast<double>(boost::trim_copy(coordinates[1]));
                    double x1 = boost::lexical_cast<double>(boost::trim_copy(coordinates[2]));
                    double y1 = boost::lexical_cast<double>(boost::trim_copy(coordinates[3]));

                    mapBounds.expandToContain(_projection->fromWgs84(MapPos(x0, y0)));
                    mapBounds.expandToContain(_projection->fromWgs84(MapPos(x1, y0)));
                    mapBounds.expandToContain(_projection->fromWgs84(MapPos(x1, y1)));
                    mapBounds.expandToContain(_projection->fromWgs84(MapPos(x0, y1)));
                    foundMapBounds = true;
                }
            }
            query.finish();
        }
        catch (const std::exception& ex) {
            Log::Errorf("MBTilesTileDataSource::getDataExtent: Exception while reading bounds metadata: %s", ex.what());
        }

        // If metadata was not available, use tiles at last zoom level
        if (!foundMapBounds) {
            try {
                sqlite3pp::query query(*_database, "SELECT MIN(tile_column), MIN(tile_row), MAX(tile_column), MAX(tile_row), MAX(zoom_level) FROM tiles WHERE zoom_level=(SELECT MAX(zoom_level) FROM tiles)");
                for (auto it = query.begin(); it != query.end(); it++) {
                    int tileX0 = (*it).get<int>(0);
                    int tileY0 = (*it).get<int>(1);
                    int tileX1 = (*it).get<int>(2) + 1;
                    int tileY1 = (*it).get<int>(3) + 1;
                    int maxZoom = (*it).get<int>(4);
                    if (_scheme == MBTilesScheme::MBTILES_SCHEME_XYZ) { // NOTE: vertically flipped
                        tileY0 = (1 << maxZoom) - 1 - tileY0;
                        tileY1 = (1 << maxZoom) - 1 - tileY1;
                        std::swap(tileY0, tileY1);
                    }

                    MapBounds projBounds = _projection->getBounds();
                    int zoomTiles = (1 << maxZoom);
                    double tileWidth  = projBounds.getDelta().getX() / zoomTiles;
                    double tileHeight = projBounds.getDelta().getY() / zoomTiles;
                    
                    MapPos tileP0(projBounds.getMin().getX() + tileX0 * tileWidth, projBounds.getMin().getY() + tileY0 * tileHeight);
                    MapPos tileP1(projBounds.getMin().getX() + tileX1 * tileWidth, projBounds.getMin().getY() + tileY1 * tileHeight);
                    
                    mapBounds.expandToContain(MapBounds(tileP0, tileP1));
                }
                query.finish();
            }
            catch (const std::exception& ex) {
                Log::Errorf("MBTilesTileDataSource::getDataExtent: Failed to query tile data from the database: %s", ex.what());
                return false;
            }
        }
        return true;
    }

}

#endif
