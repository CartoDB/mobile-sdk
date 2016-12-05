#include "CartoMapsService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "datasources/HTTPTileDataSource.h"
#include "datasources/MemoryCacheTileDataSource.h"
#include "layers/Layer.h"
#include "layers/RasterTileLayer.h"
#include "layers/VectorTileLayer.h"
#include "layers/TorqueTileLayer.h"
#include "styles/CartoCSSStyleSet.h"
#include "vectortiles/MBVectorTileDecoder.h"
#include "vectortiles/TorqueTileDecoder.h"
#include "utils/AssetPackage.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <algorithm>

#include <boost/lexical_cast.hpp>

namespace carto {

    CartoMapsService::CartoMapsService() :
        _username(),
        _apiTemplate(DEFAULT_API_TEMPLATE),
        _tilerURL(),
        _statTag(),
        _interactive(false),
        _layerIndices(),
        _layerFilter(),
        _authTokens(),
        _cdnURLs(),
        _defaultVectorLayerMode(false),
        _strictMode(false),
        _vectorTileAssetPackage(),
        _mutex()
    {
    }

    CartoMapsService::~CartoMapsService() {
    }

    std::string CartoMapsService::getUsername() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _username;
    }
        
    void CartoMapsService::setUsername(const std::string& username) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _username = username;
    }

    std::string CartoMapsService::getAPITemplate() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _apiTemplate;
    }

    void CartoMapsService::setAPITemplate(const std::string& apiTemplate) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _apiTemplate = apiTemplate;
    }

    std::string CartoMapsService::getTilerURL() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _tilerURL;
    }

    void CartoMapsService::setTilerURL(const std::string& tilerURL) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _tilerURL = tilerURL;
    }

    std::string CartoMapsService::getStatTag() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _statTag;
    }

    void CartoMapsService::setStatTag(const std::string& statTag) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _statTag = statTag;
    }

    bool CartoMapsService::isInteractive() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _interactive;
    }
    
    void CartoMapsService::setInteractive(bool interactive) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _interactive = interactive;
    }

    std::vector<int> CartoMapsService::getLayerIndices() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _layerIndices;
    }

    void CartoMapsService::setLayerIndices(const std::vector<int>& layerIndices) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _layerIndices = layerIndices;
    }

    std::vector<std::string> CartoMapsService::getLayerFilter() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _layerFilter;
    }

    void CartoMapsService::setLayerFilter(const std::vector<std::string>& filter) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _layerFilter = filter;
    }

    std::vector<std::string> CartoMapsService::getAuthTokens() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _authTokens;
    }

    void CartoMapsService::setAuthTokens(const std::vector<std::string>& authTokens) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _authTokens = authTokens;
    }

    std::map<std::string, std::string> CartoMapsService::getCDNURLs() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _cdnURLs;
    }

    void CartoMapsService::setCDNURLs(const std::map<std::string, std::string>& cdnURLs) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _cdnURLs = cdnURLs;
    }

    bool CartoMapsService::isDefaultVectorLayerMode() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _defaultVectorLayerMode;
    }

    void CartoMapsService::setDefaultVectorLayerMode(bool vectorLayerMode) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _defaultVectorLayerMode = vectorLayerMode;
    }

    bool CartoMapsService::isStrictMode() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _strictMode;
    }
    
    void CartoMapsService::setStrictMode(bool strictMode) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _strictMode = strictMode;
    }

    std::shared_ptr<AssetPackage> CartoMapsService::getVectorTileAssetPackage() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _vectorTileAssetPackage;
    }
    
    void CartoMapsService::setVectorTileAssetPackage(const std::shared_ptr<AssetPackage>& assetPackage) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _vectorTileAssetPackage = assetPackage;
    }

    std::vector<std::shared_ptr<Layer> > CartoMapsService::buildMap(const Variant& mapConfig) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        // Build URL
        std::string url = getServiceURL("/api/v1/map");

        // Create request data
        std::string mapConfigJSON = mapConfig.toString();
        auto requestData = std::make_shared<BinaryData>(reinterpret_cast<const unsigned char*>(mapConfigJSON.data()), mapConfigJSON.size());

        // Do HTTP POST request
        HTTPClient client(Log::IsShowDebug());
        std::shared_ptr<BinaryData> responseData;
        std::map<std::string, std::string> responseHeaders;
        if (client.post(url, "application/json", requestData, std::map<std::string, std::string>(), responseHeaders, responseData) != 0) {
            std::string result;
            if (responseData) {
                result = std::string(reinterpret_cast<const char*>(responseData->data()), responseData->size());
            }
            throw GenericException("Failed to read map configuration", result);
        }

        // Parse result
        std::string result(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        picojson::value mapInfo;
        std::string err = picojson::parse(mapInfo, result);
        if (!err.empty()) {
            throw ParseException("Failed to parse map configuration response", result);
        }

        // Check for errors and log them
        if (mapInfo.get("errors").is<picojson::array>()) {
            const picojson::array& errorsInfo = mapInfo.get("errors").get<picojson::array>();
            for (auto it = errorsInfo.begin(); it != errorsInfo.end(); it++) {
                Log::Errorf("CartoMapsService::buildMap: %s", it->get<std::string>().c_str());
            }
            if (!errorsInfo.empty()) {
                std::string firstError = errorsInfo.front().get<std::string>();
                throw GenericException("Errors when trying to instantiate named map", firstError);
            }
        }

        // Create layers
        return createLayers(mapInfo);
    }

    std::vector<std::shared_ptr<Layer> > CartoMapsService::buildNamedMap(const std::string& templateId, const std::map<std::string, Variant>& templateParams) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        // Build URL
        std::string url = getServiceURL("/api/v1/map/named/" + NetworkUtils::URLEncode(templateId));

        // Create request data by serializing parameters
        std::string paramsJSON = Variant(templateParams).toString();
        auto requestData = std::make_shared<BinaryData>(reinterpret_cast<const unsigned char*>(paramsJSON.data()), paramsJSON.size());

        // Perform HTTP request
        HTTPClient client(Log::IsShowDebug());
        std::shared_ptr<BinaryData> responseData;
        std::map<std::string, std::string> responseHeaders;
        if (client.post(url, "application/json", requestData, std::map<std::string, std::string>(), responseHeaders, responseData) != 0) {
            std::string result;
            if (responseData) {
                result = std::string(reinterpret_cast<const char*>(responseData->data()), responseData->size());
            }
            throw GenericException("Failed to read map configuration", result);
        }
        
        // Parse result
        std::string result(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        picojson::value mapInfo;
        std::string err = picojson::parse(mapInfo, result);
        if (!err.empty()) {
            throw ParseException("Failed to parse map configuration response", result);
        }

        // Create layers
        return createLayers(mapInfo);
    }

    std::string CartoMapsService::getServiceURL(const std::string& path) const {
        std::map<std::string, std::string> tagValues = { { "user", _username },{ "username", _username } };
        std::string url = GeneralUtils::ReplaceTags(_apiTemplate, tagValues, "{", "}", false) + path;
        std::multimap<std::string, std::string> urlParams;

        if (!_authTokens.empty()) {
            for (const std::string& authToken : _authTokens) {
                urlParams.insert({ _authTokens.size() == 1 ? "auth_token" : "auth_token[]", authToken });
            }
            url = NetworkUtils::SetURLProtocol(url, "https");
        }

        if (!_statTag.empty()) {
            urlParams.insert({ "stat_tag", _statTag });
        }

        return NetworkUtils::BuildURLFromParameters(url, urlParams);
    }

    std::string CartoMapsService::getTilerURL(const std::map<std::string, std::string>& cdnURLs) const {
        std::map<std::string, std::string> tagValues = { { "user", _username }, { "username", _username } };

        std::string apiTemplate = getAPITemplate();
        std::string url = GeneralUtils::ReplaceTags(apiTemplate, tagValues, "{", "}", false);

        std::string tilerURL = getTilerURL();
        if (!tilerURL.empty()) {
            url = GeneralUtils::ReplaceTags(tilerURL, tagValues, "{", "}", false);
        }

        std::string::size_type pos = url.find("://");
        if (pos != std::string::npos) {
            std::string proto = url.substr(0, pos);

            std::map<std::string, std::string> urls = cdnURLs;
            urls.insert(_cdnURLs.begin(), _cdnURLs.end());
            auto it = urls.find(proto);
            if (it != urls.end()) {
                if (!it->second.empty()) {
                    url = it->first + "://" + it->second + "/" + _username;
                }
            }
        }

        return url;
    }

    std::shared_ptr<Layer> CartoMapsService::createLayer(int layerIndex, const std::string& layerGroupId, const std::string& type, const std::string& layerId, const std::string& cartoCSS, const std::map<std::string, std::string>& cdnURLs) const {
        // Check layer indices filter
        if (!_layerIndices.empty()) {
            if (std::find(_layerIndices.begin(), _layerIndices.end(), layerIndex) == _layerIndices.end()) {
                return std::shared_ptr<Layer>();
            }
        }

        // Check layer type filter
        if (!_layerFilter.empty()) {
            if (std::find(_layerFilter.begin(), _layerFilter.end(), type) == _layerFilter.end()) {
                return std::shared_ptr<Layer>();
            }
        }

        // Data source parameters
        int minZoom = 0;
        int maxZoom = Const::MAX_SUPPORTED_ZOOM_LEVEL;

        std::string urlTemplateBase = getTilerURL(cdnURLs);
        urlTemplateBase += "/api/v1/map/" + layerGroupId;
        urlTemplateBase += "/" + boost::lexical_cast<std::string>(layerIndex);
        std::string urlTemplateSuffix;

        // Create layer based on type and flags
        if (type == "torque") {
            if (!cartoCSS.empty()) {
                auto baseDataSource = std::make_shared<HTTPTileDataSource>(minZoom, maxZoom, urlTemplateBase + "/{z}/{x}/{y}.torque" + urlTemplateSuffix);
                auto dataSource = std::make_shared<MemoryCacheTileDataSource>(baseDataSource); // in memory cache allows to change style quickly
                auto styleSet = std::make_shared<CartoCSSStyleSet>(cartoCSS, _vectorTileAssetPackage);
                auto torqueTileDecoder = std::make_shared<TorqueTileDecoder>(styleSet);
                return std::make_shared<TorqueTileLayer>(dataSource, torqueTileDecoder);
            }
            else {
                Log::Warn("CartoMapsService::createLayer: No CartoCSS for Torque layer, ignoring layer");
                return std::shared_ptr<Layer>();
            }
        }

        if (type == "mapnik" || type == "cartodb") {
            std::shared_ptr<TileLayer> layer;
            
            if (_defaultVectorLayerMode) {
                if (!cartoCSS.empty()) {
                    auto baseDataSource = std::make_shared<HTTPTileDataSource>(minZoom, maxZoom, urlTemplateBase + "/{z}/{x}/{y}.mvt" + urlTemplateSuffix);
                    auto dataSource = std::make_shared<MemoryCacheTileDataSource>(baseDataSource); // in memory cache allows to change style quickly
                    auto styleSet = std::make_shared<CartoCSSStyleSet>(cartoCSS, _vectorTileAssetPackage);
                    auto vectorTileDecoder = std::make_shared<MBVectorTileDecoder>(styleSet);
                    vectorTileDecoder->setFeatureIdOverride(true); // Carto uses tile-local ids for features
                    vectorTileDecoder->setCartoCSSLayerNamesIgnored(true); // all layer name filters should be ignored
                    vectorTileDecoder->setLayerNameOverride(layerId.empty() ? "layer0" : layerId);
                    layer = std::make_shared<VectorTileLayer>(dataSource, vectorTileDecoder);
                }
                else {
                    Log::Warn("CartoMapsService::createLayer: No CartoCSS for layer, using raster tiles");
                }
            }
            
            if (!layer) {
                auto dataSource = std::make_shared<HTTPTileDataSource>(minZoom, maxZoom, urlTemplateBase + "/{z}/{x}/{y}.png" + urlTemplateSuffix);
                layer = std::make_shared<RasterTileLayer>(dataSource);
            }

            if (_interactive) {
                auto dataSource = std::make_shared<HTTPTileDataSource>(minZoom, maxZoom, urlTemplateBase + "/{z}/{x}/{y}.grid.json" + urlTemplateSuffix);
                layer->setUTFGridDataSource(dataSource);
            }
            
            return layer;
        }

        auto dataSource = std::make_shared<HTTPTileDataSource>(minZoom, maxZoom, urlTemplateBase + "/{z}/{x}/{y}.png" + urlTemplateSuffix);
        return std::make_shared<RasterTileLayer>(dataSource);
    }

    std::vector<std::shared_ptr<Layer> > CartoMapsService::createLayers(const picojson::value& mapInfo) const {
        std::vector<std::shared_ptr<Layer> > layers;

        if (!mapInfo.get("layergroupid").is<std::string>()) {
            Log::Warn("CartoMapsService::createLayers: No layergroupid in response");
            return layers;
        }
        std::string layerGroupId = mapInfo.get("layergroupid").get<std::string>();

        const picojson::value& metadata = mapInfo.get("metadata");
        if (!metadata.is<picojson::object>()) {
            Log::Warn("CartoMapsService::createLayers: No metadata in response");
            return layers;
        }

        if (!metadata.get("layers").is<picojson::array>()) {
            Log::Warn("CartoMapsService::createLayers: No layers info in response");
            return layers;
        }
        const picojson::array& layersInfo = metadata.get("layers").get<picojson::array>();

        for (auto it = layersInfo.begin(); it != layersInfo.end(); it++) {
            const picojson::value& layerInfo = *it;

            // Read layer type and id
            std::string type = layerInfo.get("type").get<std::string>();
            std::string layerId;
            if (layerInfo.get("id").is<std::string>()) {
                layerId = layerInfo.get("id").get<std::string>();
            }
            
            // Read CDN URLs
            std::map<std::string, std::string> cdnURLs;
            if (mapInfo.get("cdn_url").is<picojson::object>()) {
                const picojson::object& cdnURLsObject = mapInfo.get("cdn_url").get<picojson::object>();
                for (auto it = cdnURLsObject.begin(); it != cdnURLsObject.end(); it++) {
                    if (it->second.is<std::string>()) {
                        cdnURLs[it->first] = it->second.get<std::string>();
                    }
                }
            }

            // Read CartoCSS
            std::string cartoCSS;
            if (layerInfo.get("meta").is<picojson::object>()) {
                const picojson::value& metaValue = layerInfo.get("meta");
                if (metaValue.get("cartocss").is<std::string>()) {
                    cartoCSS = metaValue.get("cartocss").get<std::string>();
                }
            }

            int layerIndex = static_cast<int>(it - layersInfo.begin());
            try {
                if (std::shared_ptr<Layer> layer = createLayer(layerIndex, layerGroupId, type, layerId, cartoCSS, cdnURLs)) {
                    layers.push_back(layer);
                }
            }
            catch (const std::exception& ex) {
                if (_strictMode) {
                    throw ex;
                }
                Log::Errorf("CartoMapsService::createLayers: Exception while creating layer: %s", ex.what());
            }
        }

        return layers;
    }

    const std::string CartoMapsService::DEFAULT_API_TEMPLATE = "http://{user}.carto.com";

}
