#include "CartoOnlineTileDataSource.h"
#include "core/MapTile.h"
#include "components/LicenseManager.h"
#include "utils/Log.h"
#include "utils/NetworkUtils.h"

#include <sstream>

namespace carto {
    
    CartoOnlineTileDataSource::CartoOnlineTileDataSource(const std::string& source) :
        TileDataSource(0, GetSourceMaxZoom(source)),
        _source(source),
        _cache(DEFAULT_CACHED_TILES),
        _httpClient(false),
        _mutex()
    {
    }
    
    CartoOnlineTileDataSource::~CartoOnlineTileDataSource() {
    }
    
    std::shared_ptr<TileData> CartoOnlineTileDataSource::loadTile(const MapTile& mapTile) {
        std::unique_lock<std::recursive_mutex> lock(_mutex);

        std::shared_ptr<TileData> tileData;
        if (_cache.read(mapTile.getTileId(), tileData)) {
            if (tileData->getMaxAge() != 0) {
                return tileData;
            }
            _cache.remove(mapTile.getTileId());
        }

        lock.unlock();
        tileData = loadOnlineTile(mapTile);
        lock.lock();
        
        if (tileData) {
            if (tileData->getMaxAge() != 0 && tileData->getData() && !tileData->isReplaceWithParent()) {
                _cache.put(mapTile.getTileId(), tileData, 1);
            }
        }
        
        return tileData;
    }
    
    std::shared_ptr<TileData> CartoOnlineTileDataSource::loadOnlineTile(const MapTile& mapTile) {
        Log::Infof("CartoOnlineTileDataSource::loadOnlineTile: Loading tile %d/%d/%d", mapTile.getZoom(), mapTile.getX(), mapTile.getY());

        std::stringstream ss;
        if (_source.substr(0, 7) == "mapzen.") {
            ss << MAPZEN_TILE_SERVICE_URL << _source.substr(7) << "/all/" << mapTile.getZoom() << "/" << mapTile.getX() << "/" << mapTile.getY() << ".mvt";
        } else {
            ss << NUTITEQ_TILE_SERVICE_URL << _source << "/" << mapTile.getZoom() << "/" << mapTile.getX() << "/" << mapTile.getY() << ".vt?user_key=" << LicenseManager::GetInstance().getUserKey();
        }
        std::string url = ss.str();
        std::map<std::string, std::string> requestHeaders;
        std::map<std::string, std::string> responseHeaders;
        std::shared_ptr<BinaryData> responseData;
        if (_httpClient.get(url, requestHeaders, responseHeaders, responseData) != 0) {
            Log::Errorf("CartoOnlineTileDataSource::loadOnlineTile: Failed to load tile %d/%d/%d", mapTile.getZoom(), mapTile.getX(), mapTile.getY());
            return std::shared_ptr<TileData>();
        }
        int maxAge = NetworkUtils::GetMaxAgeHTTPHeader(responseHeaders);
        auto tileData = std::make_shared<TileData>(responseData);
        if (maxAge >= 0) {
            tileData->setMaxAge(maxAge * 1000);
        }
        return tileData;
    }

    int CartoOnlineTileDataSource::GetSourceMaxZoom(const std::string& source) {
       if (source.substr(0, 7) == "mapzen.") {
           return 17;
       } else {
           return 14;
       }
    }

    const int CartoOnlineTileDataSource::DEFAULT_CACHED_TILES = 8;

    const std::string CartoOnlineTileDataSource::NUTITEQ_TILE_SERVICE_URL = "http://api.nutiteq.com/v1/";

    const std::string CartoOnlineTileDataSource::MAPZEN_TILE_SERVICE_URL = "http://vector.dev.mapzen.com/";
    
}
