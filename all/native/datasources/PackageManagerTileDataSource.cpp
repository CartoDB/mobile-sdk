#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "PackageManagerTileDataSource.h"
#include "core/MapTile.h"
#include "utils/Log.h"
#include "utils/Const.h"

#include <memory>

namespace carto {

    PackageManagerTileDataSource::PackageManagerTileDataSource(const std::shared_ptr<PackageManager>& packageManager) :
        TileDataSource(0, Const::MAX_SUPPORTED_ZOOM_LEVEL), _packageManager(packageManager)
    {
        if (!packageManager) {
            throw std::invalid_argument("Null packageManager");
        }

        _packageManagerListener = std::make_shared<PackageManagerListener>(*this);
        _packageManager->registerOnChangeListener(_packageManagerListener);
    }

    PackageManagerTileDataSource::~PackageManagerTileDataSource() {
        _packageManager->unregisterOnChangeListener(_packageManagerListener);
        _packageManager.reset();
    }

    std::shared_ptr<PackageManager> PackageManagerTileDataSource::getPackageManager() const {
        return _packageManager;
    }

    std::shared_ptr<TileData> PackageManagerTileDataSource::loadTile(const MapTile& mapTile) {
        Log::Infof("PackageManagerTileDataSource::loadTile: Loading %s", mapTile.toString().c_str());
        try {
            std::shared_ptr<BinaryData> data = _packageManager->loadTile(mapTile);
            std::shared_ptr<TileData> tileData = std::make_shared<TileData>(data);
            if (!data) {
                if (mapTile.getZoom() > getMinZoom()) {
                    Log::Infof("PackageManagerTileDataSource::loadTile: Tile data doesn't exist in the database, redirecting to parent.");
                    tileData->setReplaceWithParent(true);
                } else {
                    Log::Infof("PackageManagerTileDataSource::loadTile: Tile data doesn't exist in the database.");
                    return std::shared_ptr<TileData>();
                }
            }
            return tileData;
        }
        catch (const std::exception& e) {
            Log::Errorf("PackageManagerTileDataSource::loadTile: Exception: %s", e.what());
        }
        return std::shared_ptr<TileData>();
    }
        
    PackageManagerTileDataSource::PackageManagerListener::PackageManagerListener(PackageManagerTileDataSource& dataSource) :
        _dataSource(dataSource)
    {
    }
        
    void PackageManagerTileDataSource::PackageManagerListener::onTilesChanged() {
        _dataSource.notifyTilesChanged(false);
    }

}

#endif
