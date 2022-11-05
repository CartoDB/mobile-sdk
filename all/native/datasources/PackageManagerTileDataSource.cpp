#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "PackageManagerTileDataSource.h"
#include "core/MapTile.h"
#include "components/Exceptions.h"
#include "packagemanager/handlers/MapPackageHandler.h"
#include "utils/Log.h"
#include "utils/Const.h"

#include <memory>

namespace carto {

    PackageManagerTileDataSource::PackageManagerTileDataSource(const std::shared_ptr<PackageManager>& packageManager) :
        TileDataSource(0, Const::MAX_SUPPORTED_ZOOM_LEVEL),
        _packageManager(packageManager),
        _cachedOpenPackageHandlers(),
        _mutex()
    {
        if (!packageManager) {
            throw NullArgumentException("Null packageManager");
        }

        _packageManagerListener = std::make_shared<PackageManagerListener>(*this);
        _packageManager->registerOnChangeListener(_packageManagerListener);
    }

    PackageManagerTileDataSource::~PackageManagerTileDataSource() {
        _packageManager->unregisterOnChangeListener(_packageManagerListener);
        _packageManagerListener.reset();
    }

    std::shared_ptr<PackageManager> PackageManagerTileDataSource::getPackageManager() const {
        return _packageManager;
    }

    std::shared_ptr<TileData> PackageManagerTileDataSource::loadTile(const MapTile& mapTile) {
        Log::Infof("PackageManagerTileDataSource::loadTile: Loading %s", mapTile.toString().c_str());
        try {
            MapTile mapTileFlipped = mapTile.getFlipped();

            std::shared_ptr<BinaryData> data;
            _packageManager->accessLocalPackages([this, mapTileFlipped, &data](const std::map<std::shared_ptr<PackageInfo>, std::shared_ptr<PackageHandler> >& packageHandlerMap) {
                std::lock_guard<std::mutex> lock(_mutex);

                // Fast path: try already open packages
                for (auto it = _cachedOpenPackageHandlers.begin(); it != _cachedOpenPackageHandlers.end(); it++) {
                    const std::shared_ptr<PackageInfo>& packageInfo = it->first;
                    std::shared_ptr<PackageTileMask> tileMask = packageInfo->getTileMask();
                    if (tileMask) {
                        if (tileMask->getTileStatus(mapTileFlipped) == PackageTileStatus::PACKAGE_TILE_STATUS_MISSING) {
                            continue;
                        }
                    }

                    data = it->second->loadTile(mapTileFlipped);
                    if (data || tileMask) {
                        std::rotate(_cachedOpenPackageHandlers.begin(), it, it + 1);
                        return;
                    }
                }

                // Slow path: try other packages
                for (auto it = packageHandlerMap.begin(); it != packageHandlerMap.end(); it++) {
                    if (auto mapHandler = std::dynamic_pointer_cast<MapPackageHandler>(it->second)) {
                        const std::shared_ptr<PackageInfo>& packageInfo = it->first;
                        std::shared_ptr<PackageTileMask> tileMask = packageInfo->getTileMask();
                        if (tileMask) {
                            if (tileMask->getTileStatus(mapTileFlipped) == PackageTileStatus::PACKAGE_TILE_STATUS_MISSING) {
                                continue;
                            }
                        }

                        data = mapHandler->loadTile(mapTileFlipped);
                        if (data || tileMask) {
                            _cachedOpenPackageHandlers.insert(_cachedOpenPackageHandlers.begin(), std::make_pair(packageInfo, mapHandler));
                            if (_cachedOpenPackageHandlers.size() > MAX_OPEN_PACKAGES) {
                                _cachedOpenPackageHandlers.pop_back();
                            }
                            return;
                        }
                    }
                }
            });

            std::shared_ptr<TileData> tileData = std::make_shared<TileData>(data);
            if (!data) {
                if (mapTileFlipped.getZoom() > getMinZoom()) {
                    Log::Infof("PackageManagerTileDataSource::loadTile: Tile data doesn't exist in the database, redirecting to parent");
                    tileData->setReplaceWithParent(true);
                } else {
                    Log::Infof("PackageManagerTileDataSource::loadTile: Tile data doesn't exist in the database");
                    return std::shared_ptr<TileData>();
                }
            }
            return tileData;
        }
        catch (const std::exception& ex) {
            Log::Errorf("PackageManagerTileDataSource::loadTile: Exception: %s", ex.what());
        }
        return std::shared_ptr<TileData>();
    }
        
    PackageManagerTileDataSource::PackageManagerListener::PackageManagerListener(PackageManagerTileDataSource& dataSource) :
        _dataSource(dataSource)
    {
    }
        
    void PackageManagerTileDataSource::PackageManagerListener::onPackagesChanged(PackageChangeType changeType) {
        {
            std::lock_guard<std::mutex> lock(_dataSource._mutex);
            _dataSource._cachedOpenPackageHandlers.clear();
        }
        _dataSource.notifyTilesChanged(changeType == PACKAGES_DELETED); // we need to remove tiles only if packages were deleted
    }

    void PackageManagerTileDataSource::PackageManagerListener::onStylesChanged() {
        // NOTE: ignore
    }

    const unsigned int PackageManagerTileDataSource::MAX_OPEN_PACKAGES = 4;

}

#endif
