#include "HTTPTileDataSource.h"
#include "core/MapTile.h"
#include "utils/Log.h"
#include "utils/NetworkUtils.h"
#include "utils/GeneralUtils.h"

namespace carto {

    HTTPTileDataSource::HTTPTileDataSource(int minZoom, int maxZoom, const std::string& baseURL) :
        TileDataSource(minZoom, maxZoom),
        _baseURL(baseURL),
        _subdomains({ "a", "b", "c", "d" }),
        _tmsScheme(false),
        _maxAgeHeaderCheck(false),
        _headers(),
        _httpClient(true),
        _randomGenerator(),
        _mutex()
    {
    }
    
    HTTPTileDataSource::~HTTPTileDataSource() {
    }
    
    std::string HTTPTileDataSource::getBaseURL() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _baseURL;
    }
    
    void HTTPTileDataSource::setBaseURL(const std::string& baseURL) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _baseURL = baseURL;
        }
        notifyTilesChanged(false);
    }
    
    std::vector<std::string> HTTPTileDataSource::getSubdomains() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _subdomains;
    }
    
    void HTTPTileDataSource::setSubdomains(const std::vector<std::string>& subdomains) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _subdomains = subdomains;
        }
        notifyTilesChanged(false);
    }
    
    bool HTTPTileDataSource::isTMSScheme() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _tmsScheme;
    }
    
    void HTTPTileDataSource::setTMSScheme(bool tmsScheme) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _tmsScheme = tmsScheme;
        }
        notifyTilesChanged(false);
    }
    
    bool HTTPTileDataSource::isMaxAgeHeaderCheck() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _maxAgeHeaderCheck;
    }
    
    void HTTPTileDataSource::setMaxAgeHeaderCheck(bool maxAgeCheck) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _maxAgeHeaderCheck = maxAgeCheck;
        }
        notifyTilesChanged(false);
    }
    
    std::map<std::string, std::string> HTTPTileDataSource::getHTTPHeaders() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _headers;
    }
    
    void HTTPTileDataSource::setHTTPHeaders(const std::map<std::string, std::string>& headers) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _headers = headers;
        }
        notifyTilesChanged(false);
    }
    
    std::shared_ptr<TileData> HTTPTileDataSource::loadTile(const MapTile& mapTile) {
        std::string baseURL;
        std::map<std::string, std::string> headers;
        bool maxAgeHeaderCheck;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            baseURL = _baseURL;
            headers = _headers;
            maxAgeHeaderCheck = _maxAgeHeaderCheck;
        }
        std::string url = buildTileURL(baseURL, mapTile);
        Log::Infof("HTTPTileDataSource::loadTile: Loading %s", url.c_str());
        std::map<std::string, std::string> requestHeaders;
        std::map<std::string, std::string> responseHeaders;
        std::shared_ptr<BinaryData> responseData;
        try {
            if (_httpClient.get(url, requestHeaders, responseHeaders, responseData) != 0) {
                Log::Errorf("HTTPTileDataSource::loadTile: Failed to load %s", url.c_str());
                return std::shared_ptr<TileData>();
            }
        } catch (const std::exception& ex) {
            Log::Errorf("HTTPTileDataSource::loadTile: Exception while loading tile %d/%d/%d: %s", mapTile.getZoom(), mapTile.getX(), mapTile.getY(), ex.what());
            return std::shared_ptr<TileData>();
        }
        auto tileData = std::make_shared<TileData>(responseData);
        if (maxAgeHeaderCheck) {
            int maxAge = NetworkUtils::GetMaxAgeHTTPHeader(responseHeaders);
            if (maxAge >= 0) {
                tileData->setMaxAge(maxAge * 1000);
            }
        }
        return tileData;
    }
    
    std::string HTTPTileDataSource::buildTileURL(const std::string& baseURL, const MapTile& tile) const {
        bool tmsScheme = false;
        std::string subdomain;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            tmsScheme = _tmsScheme;
            if (!_subdomains.empty()) {
                std::size_t randomIndex = std::uniform_int_distribution<std::size_t>(0, _subdomains.size() - 1)(_randomGenerator);
                subdomain = _subdomains[randomIndex];
            }
        }

        std::map<std::string, std::string> tagValues = buildTagValues(tmsScheme ? tile.getFlipped() : tile);
        if (!subdomain.empty()) {
            tagValues["s"] = subdomain;
        }
   
        return GeneralUtils::ReplaceTags(baseURL, tagValues, "{", "}", true);
    }
    
}
