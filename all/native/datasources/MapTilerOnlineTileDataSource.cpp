#include "MapTilerOnlineTileDataSource.h"
#include "core/BinaryData.h"
#include "core/MapTile.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/PlatformUtils.h"

#include <picojson/picojson.h>

namespace carto {
    
    MapTilerOnlineTileDataSource::MapTilerOnlineTileDataSource(const std::string& key) :
        TileDataSource(),
        _key(key),
        _httpClient(Log::IsShowDebug()),
        _serviceURL(),
        _tmsScheme(false),
        _tileURLs(),
        _randomGenerator(),
        _mutex()
    {
        _maxZoom = DEFAULT_MAX_ZOOM;
    }
    
    MapTilerOnlineTileDataSource::~MapTilerOnlineTileDataSource() {
    }

    std::string MapTilerOnlineTileDataSource::getCustomServiceURL() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _serviceURL;
    }

    void MapTilerOnlineTileDataSource::setCustomServiceURL(const std::string& serviceURL) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _serviceURL = serviceURL;
    }

    std::shared_ptr<TileData> MapTilerOnlineTileDataSource::loadTile(const MapTile& mapTile) {
        std::unique_lock<std::recursive_mutex> lock(_mutex);

        // Reload tile service URLs, if needed
        if (_tileURLs.empty()) {
            if (!loadConfiguration()) {
                return std::shared_ptr<TileData>();
            }
        }

        // Select tile URL randomly
        std::size_t randomIndex = std::uniform_int_distribution<std::size_t>(0, _tileURLs.size() - 1)(_randomGenerator);
        std::string tileURL = _tileURLs[randomIndex];

        // Fetch online tile, allow parallel tile fetching
        lock.unlock();
        std::shared_ptr<TileData> tileData = loadOnlineTile(tileURL, mapTile);
        lock.lock();

        return tileData;
    }

    std::string MapTilerOnlineTileDataSource::buildTileURL(const std::string& baseURL, const MapTile& tile) const {
        std::map<std::string, std::string> tagValues = buildTagValues(_tmsScheme ? tile.getFlipped() : tile);
        return GeneralUtils::ReplaceTags(baseURL, tagValues, "{", "}", true);
    }

    bool MapTilerOnlineTileDataSource::loadConfiguration() {
        std::map<std::string, std::string> tagMap;
        tagMap["key"] = _key;
        std::string url = GeneralUtils::ReplaceTags(_serviceURL.empty() ? MAPTILER_SERVICE_URL : _serviceURL, tagMap);
        Log::Debugf("MapTilerOnlineTileDataSource::loadConfiguration: Loading %s", url.c_str());

        std::map<std::string, std::string> requestHeaders = NetworkUtils::CreateAppRefererHeader();
        std::map<std::string, std::string> responseHeaders;
        std::string responseString;
        if (!NetworkUtils::GetHTTP(url, requestHeaders, responseHeaders, responseString, Log::IsShowDebug())) {
            Log::Warnf("MapTilerOnlineTileDataSource: Failed to fetch tile source configuration");
            if (responseString.empty()) { // NOTE: we may have error messages, thus do not return from here unless empty data
                return false;
            }
        }

        picojson::value config;
        std::string err = picojson::parse(config, responseString);
        if (!err.empty()) {
            Log::Errorf("MapTilerOnlineTileDataSource: Failed to parse tile source configuration: %s", err.c_str());
            return false;
        }

        _tileURLs.clear();
        if (!config.get("tiles").is<picojson::array>()) {
            Log::Error("MapTilerOnlineTileDataSource: Tile URLs missing from configuration");
            return false;
        }
        for (const picojson::value& tileURL : config.get("tiles").get<picojson::array>()) {
            if (tileURL.is<std::string>()) {
                _tileURLs.push_back(tileURL.get<std::string>());
            }
        }
        _tmsScheme = false;
        if (config.get("scheme").is<std::string>()) {
            _tmsScheme = config.get("scheme").get<std::string>() == "tms";
        }
        if (config.get("minzoom").is<std::int64_t>()) {
            int minZoom = static_cast<int>(config.get("minzoom").get<std::int64_t>());
            if (minZoom != _minZoom) {
                _minZoom = minZoom;
                notifyTilesChanged(false);
            }
        }
        if (config.get("maxzoom").is<std::int64_t>()) {
            int maxZoom = static_cast<int>(config.get("maxzoom").get<std::int64_t>());
            if (maxZoom != _maxZoom) {
                _maxZoom = maxZoom;
                notifyTilesChanged(false);
            }
        }
        return !_tileURLs.empty();
    }
    
    std::shared_ptr<TileData> MapTilerOnlineTileDataSource::loadOnlineTile(const std::string& tileURL, const MapTile& mapTile) {
        Log::Infof("MapTilerOnlineTileDataSource::loadOnlineTile: Loading tile %d/%d/%d", mapTile.getZoom(), mapTile.getX(), mapTile.getY());

        std::string url = buildTileURL(tileURL, mapTile);
        if (url.empty()) {
            return std::shared_ptr<TileData>();
        }

        Log::Debugf("MapTilerOnlineTileDataSource::loadOnlineTile: Loading %s", url.c_str());
        std::map<std::string, std::string> requestHeaders = NetworkUtils::CreateAppRefererHeader();
        std::map<std::string, std::string> responseHeaders;
        std::shared_ptr<BinaryData> responseData;
        int statusCode = -1;
        try {
            if (_httpClient.get(url, requestHeaders, responseHeaders, responseData, &statusCode) != 0) {
                if (statusCode == 404) {
                    responseData = std::make_shared<BinaryData>(std::vector<unsigned char>());
                } else {
                    Log::Errorf("MapTilerOnlineTileDataSource::loadOnlineTile: Failed to load tile %d/%d/%d: status code %d", mapTile.getZoom(), mapTile.getX(), mapTile.getY(), statusCode);
                    return std::shared_ptr<TileData>();
                }
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("MapTilerOnlineTileDataSource::loadOnlineTile: Exception while loading tile %d/%d/%d: %s", mapTile.getZoom(), mapTile.getX(), mapTile.getY(), ex.what());
            return std::shared_ptr<TileData>();
        }
        int maxAge = NetworkUtils::GetMaxAgeHTTPHeader(responseHeaders);
        auto tileData = std::make_shared<TileData>(responseData);
        if (maxAge > 0) {
            Log::Infof("MapTilerOnlineTileDataSource::loadOnlineTile: Setting tile %d/%d/%d maxage=%d", mapTile.getZoom(), mapTile.getX(), mapTile.getY(), maxAge);
            tileData->setMaxAge(maxAge * 1000);
        }
        if (statusCode == 204) { // special case - empty tile, replace with parent tile
            Log::Infof("MapTilerOnlineTileDataSource::loadOnlineTile: Replacing tile %d/%d/%d with parent", mapTile.getZoom(), mapTile.getX(), mapTile.getY());
            tileData->setReplaceWithParent(true);
        }
        return tileData;
    }

    const int MapTilerOnlineTileDataSource::DEFAULT_MAX_ZOOM = 14;

    const std::string MapTilerOnlineTileDataSource::MAPTILER_SERVICE_URL = "https://api.maptiler.com/tiles/v3/tiles.json?key={key}";
    
}
