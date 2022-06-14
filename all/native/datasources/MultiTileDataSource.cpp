#include "MultiTileDataSource.h"
#include "core/MapTile.h"
#include "components/Exceptions.h"
#include "utils/GeneralUtils.h"
#include "utils/Log.h"
#include "utils/Const.h"
#include "packagemanager/PackageTileMask.h"
#include "datasources/MBTilesTileDataSource.h"

#include <boost/lexical_cast.hpp>

#include <memory>

namespace carto {

    MultiTileDataSource::MultiTileDataSource(int maxOpenedPackages) : TileDataSource(0, Const::MAX_SUPPORTED_ZOOM_LEVEL),
                                                                                                  _dataSources(),
                                                                                                  _cachedOpenDataSources(),
                                                                                                  _maxOpenedPackages(maxOpenedPackages),
                                                                                                  _mutex()
    {
    }
    MultiTileDataSource::MultiTileDataSource() : TileDataSource(0, Const::MAX_SUPPORTED_ZOOM_LEVEL),
                                                                             _dataSources(),
                                                                             _cachedOpenDataSources(),
                                                                             _maxOpenedPackages(4),
                                                                             _mutex()
    {
    }

    MultiTileDataSource::~MultiTileDataSource() {
    }

    bool compare_datasource (std::pair<std::shared_ptr<PackageTileMask>, std::shared_ptr<TileDataSource>> dataSource1, std::pair<std::shared_ptr<PackageTileMask>, std::shared_ptr<TileDataSource>> dataSource2) {
        return std::dynamic_pointer_cast<TileDataSource>(dataSource1.second) == std::dynamic_pointer_cast<TileDataSource>(dataSource2.second);
    };

    std::shared_ptr<TileData> MultiTileDataSource::loadTile(const MapTile &mapTile)
    {
        Log::Infof("MultiTileDataSource::loadTile: Loading %s", mapTile.toString().c_str());
        try
        {
            MapTile mapTileFlipped = mapTile.getFlipped();

            std::shared_ptr<TileData> tileData;
            std::lock_guard<std::mutex> lock(_mutex);
            bool tileOk = false;
            const int zoom = mapTile.getZoom();
            // Fast path: try already open packages
            for (auto it = _cachedOpenDataSources.begin(); it != _cachedOpenDataSources.end(); it++)
            {
                auto dataSource = it->second;
                if (zoom < dataSource->getMinZoom() || zoom > dataSource->getMaxZoom()) {
                    continue;
                }

                std::shared_ptr<PackageTileMask> tileMask = it->first;
                if (tileMask)
                {
                    if (tileMask->getTileStatus(mapTileFlipped) == PackageTileStatus::PACKAGE_TILE_STATUS_MISSING)
                    {
                        continue;
                    }
                }

                tileData = dataSource->loadTile(mapTile);
                tileOk = tileData && tileData->getData();
                if (tileOk)
                {
                    std::rotate(_cachedOpenDataSources.begin(), it, it + 1);
                    break;
                }
            }
            if (!tileOk)
            {
                // Slow path: try other packages
                for (auto it = _dataSources.begin(); it != _dataSources.end(); it++)
                {

                    if (auto dataSource = std::dynamic_pointer_cast<TileDataSource>(it->second))
                    {
                        auto it2 = std::find_if(_cachedOpenDataSources.begin(), _cachedOpenDataSources.end(), [&it](const std::pair<std::shared_ptr<PackageTileMask>, std::shared_ptr<TileDataSource>> pair)
                        {  return pair.second == it->second; });
                        if (it2 != _cachedOpenDataSources.end() && it2->second == it->second) {
                            continue;
                        }
                        if (zoom < dataSource->getMinZoom() || zoom > dataSource->getMaxZoom()) {
                            continue;
                        }
                        std::shared_ptr<PackageTileMask> tileMask = it->first;
                        if (tileMask)
                        {
                            if (tileMask->getTileStatus(mapTileFlipped) == PackageTileStatus::PACKAGE_TILE_STATUS_MISSING)
                            {
                                continue;
                            }
                        }

                        tileData = dataSource->loadTile(mapTile);
                        tileOk = tileData && tileData->getData();
                        if (tileOk) {
                            _cachedOpenDataSources.insert(_cachedOpenDataSources.begin(), std::make_pair(tileMask, dataSource));
                            if (_cachedOpenDataSources.size() > _maxOpenedPackages)
                            {
                                _cachedOpenDataSources.pop_back();
                            }
                            break;
                        }
                    }
                }
            }

            if (!tileOk)
            {

                if (mapTile.getZoom() > getMinZoom())
                {
                    Log::Infof("MultiTileDataSource::loadTile: Tile data doesn't exist in the database, redirecting to parent");
                    if (!tileData){
                        tileData = std::make_shared<TileData>(std::shared_ptr<BinaryData>());
                    }
                    tileData->setReplaceWithParent(true);
                }
                else
                {
                    Log::Infof("MultiTileDataSource::loadTile: Tile data doesn't exist in the database");
                    return std::shared_ptr<TileData>();
                }
            }
            return tileData;
        }
        catch (const std::exception &ex)
        {
            Log::Errorf("PackageManagerTileDataSource::loadTile: Exception: %s", ex.what());
        }
        return std::shared_ptr<TileData>();
    }

    void MultiTileDataSource::onPackagesChanged(ChangeType changeType)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _cachedOpenDataSources.clear();
        }
        notifyTilesChanged(changeType == PACKAGES_DELETED); // we need to remove tiles only if packages were deleted
    }

    void MultiTileDataSource::add(const std::shared_ptr<TileDataSource> &dataSource)
    {
        add(dataSource, std::string());
    }

    void MultiTileDataSource::add(const std::shared_ptr<TileDataSource> &dataSource, const std::string& tileMaskArg)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = std::find_if(_dataSources.begin(), _dataSources.end(), [&dataSource](const std::pair<std::shared_ptr<PackageTileMask>, std::shared_ptr<TileDataSource>> pair)
            { return pair.second == dataSource; });
            if (it != _dataSources.end()) {
                return;
            }
            std::shared_ptr<PackageTileMask> tileMask;
            std::string tileMaskStr = tileMaskArg;
            if (tileMaskStr.empty()) {
                if (auto mbtilesDatasource = std::dynamic_pointer_cast<MBTilesTileDataSource>(dataSource)) {
                    tileMaskStr = mbtilesDatasource->getMetaData("tilemask");
                }
            }
            if (!tileMaskStr.empty())
            {
                std::vector<std::string> parts = GeneralUtils::Split(tileMaskStr, ':');
                if (!parts.empty())
                {
                    int zoomLevel;
                    if (parts.size() > 1)
                    {
                        zoomLevel = boost::lexical_cast<int>(parts[1]);
                    }
                    else
                    {
                        zoomLevel = dataSource->getMaxZoom();
                    }
                    tileMask = std::make_shared<PackageTileMask>(parts[0], zoomLevel);
                }
            }
            _dataSources.emplace_back(tileMask, dataSource);
        }
        onPackagesChanged(PACKAGES_ADDED);
    }


    bool MultiTileDataSource::remove(const std::shared_ptr<TileDataSource> &dataSource)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = std::remove_if(_dataSources.begin(), _dataSources.end(), [&dataSource](
                    const std::pair<std::shared_ptr<PackageTileMask>, std::shared_ptr<TileDataSource>> pair) {
                return pair.second == dataSource;
            });
            if (it == _dataSources.end()) {
                return false;
            }
            _dataSources.erase(it);
        }
        onPackagesChanged(PACKAGES_DELETED);
        return true;
    }
}
