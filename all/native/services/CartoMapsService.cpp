#include "CartoMapsService.h"
#include "core/BinaryData.h"
#include "datasources/HTTPTileDataSource.h"
#include "layers/Layers.h"
#include "layers/RasterTileLayer.h"
#include "layers/VectorTileLayer.h"
#include "layers/TorqueTileLayer.h"
#include "vectortiles/MBVectorTileDecoder.h"
#include "vectortiles/TorqueTileDecoder.h"
#include "vectortiles/CartoCSSStyleSet.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <algorithm>
#include <regex>

#include <boost/algorithm/string.hpp>

#include <picojson/picojson.h>

namespace carto {

    CartoMapsService::CartoMapsService() :
        _username(),
        _apiKey(),
        _apiTemplate(DEFAULT_API_TEMPLATE),
        _authToken(),
        _defaultLayerType(LAYER_TYPE_RASTER),
        _layerTypes(),
        _mutex(std::make_shared<std::recursive_mutex>())
    {
    }

    CartoMapsService::~CartoMapsService() {
    }

    std::string CartoMapsService::getUsername() const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        return _username;
    }
        
    void CartoMapsService::setUsername(const std::string& username) {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        _username = username;
    }

    std::string CartoMapsService::getAPIKey() const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        return _apiKey;
    }

    void CartoMapsService::setAPIKey(const std::string& apiKey) {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        _apiKey = apiKey;
    }

    std::string CartoMapsService::getAPITemplate() const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        return _apiTemplate;
    }

    void CartoMapsService::setAPITemplate(const std::string& apiTemplate) {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        _apiTemplate = apiTemplate;
    }

    std::string CartoMapsService::getAuthToken() const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        return _authToken;
    }

    void CartoMapsService::setAuthToken(const std::string& authToken) {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        _authToken = authToken;
    }

    bool CartoMapsService::isDefaultVectorLayerMode() const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        return _defaultLayerType == LAYER_TYPE_VECTOR;
    }

    void CartoMapsService::setDefaultVectorLayerMode(bool enabled) {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        _defaultLayerType = (enabled ? LAYER_TYPE_VECTOR : LAYER_TYPE_RASTER);
    }

    bool CartoMapsService::isVectorLayerMode(int index) const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        auto it = _layerTypes.find(index);
        if (it == _layerTypes.end()) {
            return _defaultLayerType == LAYER_TYPE_VECTOR;
        }
        return it->second == LAYER_TYPE_VECTOR;
    }

    void CartoMapsService::setVectorLayerMode(int index, bool enabled) {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        _layerTypes[index] = (enabled ? LAYER_TYPE_VECTOR : LAYER_TYPE_RASTER);
    }

    void CartoMapsService::buildNamedMap(const std::shared_ptr<Layers>& layers, const std::string& templateId, const std::map<std::string, Variant>& templateParams) const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);

        // Build URL
        std::string url = getAPITemplate(picojson::value());
        url += "/api/v1/map/named/" + NetworkUtils::URLEncode(templateId);

        std::map<std::string, std::string> urlParams;
        if (!_apiKey.empty()) {
            urlParams["api_key"] = _apiKey;
            url = NetworkUtils::SetURLProtocol(url, "https");
        }
        if (!_authToken.empty()) {
            urlParams["auth_token"] = _authToken;
            url = NetworkUtils::SetURLProtocol(url, "https");
        }
        url = NetworkUtils::BuildURLFromParameters(url, urlParams);

        // Perform HTTP request
        HTTPClient client(false);
        std::shared_ptr<BinaryData> responseData;
        std::map<std::string, std::string> responseHeaders;
        if (client.get(url, std::map<std::string, std::string>(), responseHeaders, responseData) != 0) {
            std::string result;
            if (responseData) {
                result = std::string(reinterpret_cast<const char*>(responseData->data()), responseData->size());
            }
            Log::Errorf("CartoMapsService::buildNamedMap: Failed to read map configuration: %s", result.c_str());
            return;
        }
        
        // Read MapConfig
        std::string result(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        picojson::value mapConfig;
        std::string err = picojson::parse(mapConfig, result);
        if (!err.empty()) {
            Log::Errorf("CartoMapsService::buildNamedMap: Failed to parse map configuration: %s", err.c_str());
            return;
        }

        // Update placeholders
        if (mapConfig.contains("placeholders")) {
            picojson::object placeholders = mapConfig.get("placeholders").get<picojson::object>();
            replaceTemplateParams(mapConfig, placeholders, templateParams);
        }

        // Build map
        buildMap(layers, Variant::FromPicoJSON(mapConfig));
    }

    void CartoMapsService::buildMap(const std::shared_ptr<Layers>& layers, const Variant& mapConfig) const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);

        // Build URL
        std::string url = getAPITemplate(mapConfig.toPicoJSON());
        url += "/api/v1/map";
        std::map<std::string, std::string> urlParams;
        if (!_apiKey.empty()) {
            urlParams["api_key"] = _apiKey;
        }
        if (!_authToken.empty()) {
            urlParams["auth_token"] = _authToken;
        }
        url = NetworkUtils::BuildURLFromParameters(url, urlParams);

        // Do HTTP POST request
        std::string mapConfigJSON = mapConfig.toString();
        HTTPClient client(false);
        auto requestData = std::make_shared<BinaryData>(reinterpret_cast<const unsigned char*>(mapConfigJSON.data()), mapConfigJSON.size());
        std::shared_ptr<BinaryData> responseData;
        std::map<std::string, std::string> responseHeaders;
        if (client.post(url, "application/json", requestData, std::map<std::string, std::string>(), responseHeaders, responseData) != 0) {
            std::string result;
            if (responseData) {
                result = std::string(reinterpret_cast<const char*>(responseData->data()), responseData->size());
            }
            Log::Errorf("CartoMapsService::buildMap: Failed to read map configuration: %s", result.c_str());
            return;
        }

        // Get layergroupid from the response
        std::string result(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        picojson::value mapInfo;
        std::string err = picojson::parse(mapInfo, result);
        if (!err.empty()) {
            Log::Errorf("CartoMapsService::buildMap: Failed to parse response: %s", err.c_str());
            return;
        }
        std::string layerGroupId = mapInfo.get("layergroupid").get<std::string>();

        // Build layers
        const picojson::array& layerConfigs = mapConfig.toPicoJSON().get("layers").get<picojson::array>();
        for (auto it = layerConfigs.begin(); it != layerConfigs.end(); it++) {
            createLayer(layers, mapConfig.toPicoJSON(), *it, layerGroupId);
        }
    }

    int CartoMapsService::getMinZoom(const picojson::value& mapConfig) const {
        const picojson::value& minZoom = mapConfig.get("minzoom");
        if (!minZoom.is<std::int64_t>()) {
            return 0;
        }
        return static_cast<int>(minZoom.get<std::int64_t>());
    }

    int CartoMapsService::getMaxZoom(const picojson::value& mapConfig) const {
        const picojson::value& maxZoom = mapConfig.get("maxzoom");
        if (!maxZoom.is<std::int64_t>()) {
            return Const::MAX_SUPPORTED_ZOOM_LEVEL;
        }
        return static_cast<int>(maxZoom.get<std::int64_t>());
    }

    std::string CartoMapsService::getUsername(const picojson::value& mapConfig) const {
        const picojson::value& username = mapConfig.get("user_name");
        if (!username.is<std::string>()) {
            return _username;
        }
        return username.get<std::string>();
    }

    std::string CartoMapsService::getAPITemplate(const picojson::value& mapConfig) const {
        std::string username = getUsername(mapConfig);
        std::map<std::string, std::string> tagValues = { { "user", username }, { "username", username } };
        return GeneralUtils::ReplaceTags(_apiTemplate, tagValues, "{", "}", false);
    }

    void CartoMapsService::createLayer(const std::shared_ptr<Layers>& layers, const picojson::value& mapConfig, const picojson::value& layerConfig, const std::string& layerGroupId) const {
        // TODO: layer filter
        std::string type = layerConfig.get("type").get<std::string>();
        const picojson::value& options = layerConfig.get("options");
        if (type == "plain") {
            // TODO: if layers empty and plain color, set background color
            // Otherwise create RasterTileLayer with constant-color tiles?
            // Warn if imageUrl is defined
        }
        else if (type == "mapnik" || type == "cartodb" || type == "torque") {
            // TODO: use cdn parameter?
            std::string urlTemplateBase = getAPITemplate(mapConfig);
            urlTemplateBase += "/api/v1/map/" + layerGroupId + "/{z}/{x}/{y}";
            LayerType layerType = _defaultLayerType;
            auto it = _layerTypes.find(layers->count());
            if (it != _layerTypes.end()) {
                layerType = it->second;
            }
            if (type == "torque") {
                std::string cartoCSS = options.get("cartocss").get<std::string>();
                auto dataSource = std::make_shared<HTTPTileDataSource>(getMinZoom(mapConfig), getMaxZoom(mapConfig), urlTemplateBase + ".torque");
                // TODO: AssetPackage support
                auto styleSet = std::make_shared<CartoCSSStyleSet>(cartoCSS);
                auto torqueTileDecoder = std::make_shared<TorqueTileDecoder>(styleSet);
                auto layer = std::make_shared<TorqueTileLayer>(dataSource, torqueTileDecoder);
                layers->add(layer);
            }
            else if (layerType == LAYER_TYPE_VECTOR) {
                std::string cartoCSS = options.get("cartocss").get<std::string>();
                auto dataSource = std::make_shared<HTTPTileDataSource>(getMinZoom(mapConfig), getMaxZoom(mapConfig), urlTemplateBase + ".mvt");
                // TODO: AssetPackage support
                auto styleSet = std::make_shared<CartoCSSStyleSet>(cartoCSS);
                auto vectorTileDecoder = std::make_shared<MBVectorTileDecoder>(styleSet);
                auto layer = std::make_shared<VectorTileLayer>(dataSource, vectorTileDecoder);
                layers->add(layer);
            }
            else {
                auto dataSource = std::make_shared<HTTPTileDataSource>(getMinZoom(mapConfig), getMaxZoom(mapConfig), urlTemplateBase + ".png");
                auto layer = std::make_shared<RasterTileLayer>(dataSource);
                layers->add(layer);
            }
        }
        else if (type == "http") {
            std::string urlTemplate = options.get("urlTemplate").get<std::string>();
            auto dataSource = std::make_shared<HTTPTileDataSource>(getMinZoom(mapConfig), getMaxZoom(mapConfig), urlTemplate);
            std::vector<std::string> subdomains = { "a", "b", "c" };
            if (options.contains("subdomains")) {
                const picojson::array& subdomainsOption = options.get("subdomains").get<picojson::array>();
                subdomains.clear();
                std::transform(subdomainsOption.begin(), subdomainsOption.end(), std::back_inserter(subdomains), [](const picojson::value& subdomain) {
                    return subdomain.get<std::string>();
                });
            }
            dataSource->setSubdomains(subdomains);
            bool tmsScheme = false;
            if (options.contains("tms")) {
                tmsScheme = options.get("tms").get<bool>();
            }
            dataSource->setTMSScheme(tmsScheme);

            auto layer = std::make_shared<RasterTileLayer>(dataSource);
            layers->add(layer);
        }
        else if (type == "named") {
            std::string name = options.get("name").get<std::string>();
            const picojson::object& config = options.get("config").get<picojson::object>();
            std::map<std::string, Variant> params;
            for (auto it = config.begin(); it != config.end(); it++) {
                params[it->first] = Variant::FromPicoJSON(it->second);
            }
            std::vector<std::string> authTokens;
            if (options.contains("auth_tokens")) {
                const picojson::array& authTokensOption = options.get("auth_tokens").get<picojson::array>();
                std::transform(authTokensOption.begin(), authTokensOption.end(), std::back_inserter(authTokens), [](const picojson::value& authToken) {
                    return authToken.get<std::string>();
                });
            }

            auto mapsService = std::make_shared<CartoMapsService>(*this);
            if (!authTokens.empty()) {
                mapsService->setAuthToken(authTokens.front()); // TODO: what to do if more than 1?
            }
            else {
                mapsService->setAuthToken("");
            }
            mapsService->buildNamedMap(layers, name, params);
        }
        else if (type == "layergroup") {
            buildMap(layers, Variant::FromPicoJSON(options));
        }
    }

    std::string CartoMapsService::escapeParam(const std::string& value, const std::string& type) {
        if (type == "sql_literal") {
            return boost::replace_all_copy(boost::replace_all_copy(value, "\\", "\\\\"), "'", "\\'");
        }
        else if (type == "sql_ident") {
            return boost::replace_all_copy(boost::replace_all_copy(value, "\\", "\\\\"), "\"", "\\\"");
        }
        else if (type == "number") {
            if (std::regex_match(value, std::regex("^(\\-|\\+)?[0-9]*(\\.[0-9]+)?"))) {
                return value;
            }
        }
        else if (type == "css_color") {
            if (std::regex_match(value, std::regex("^(#([0-9a-fA-F]{3}){1,2}|[a-zA-Z]+)?"))) {
                return value;
            }
        }
        return std::string();
    }

    void CartoMapsService::replaceTemplateTags(picojson::value& value, const std::map<std::string, std::string>& tagValues) {
        if (value.is<std::string>()) {
            value = picojson::value(picojson::value(GeneralUtils::ReplaceTags(value.get<std::string>(), tagValues, "<%=", "%>")));
        }
        else if (value.is<picojson::array>()) {
            picojson::array& arr = value.get<picojson::array>();
            std::for_each(arr.begin(), arr.end(), [&tagValues](picojson::value& val) {
                replaceTemplateTags(val, tagValues);
            });
        }
        else if (value.is<picojson::object>()) {
            picojson::object& obj = value.get<picojson::object>();
            std::for_each(obj.begin(), obj.end(), [&tagValues](std::pair<const std::string, picojson::value>& keyVal) {
                replaceTemplateTags(keyVal.second, tagValues);
            });
        }
    }

    void CartoMapsService::replaceTemplateParams(picojson::value& templateObject, const picojson::object& placeholders, const std::map<std::string, Variant>& templateParams) {
        std::map<std::string, std::string> tagValues;
        for (auto it = placeholders.begin(); it != placeholders.end(); it++) {
            std::string value = it->second.get("default").serialize();
            auto pit = templateParams.find(it->first);
            if (pit != templateParams.end()) {
                value = pit->second.toString();
            }
            tagValues[it->first] = escapeParam(value, it->second.get("type").get<std::string>());
        }
        replaceTemplateTags(templateObject, tagValues);
    }

    const std::string CartoMapsService::DEFAULT_API_TEMPLATE = "http://{user}.cartodb.com";

}
