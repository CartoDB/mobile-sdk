#include "CartoMapsService.h"
#include "core/BinaryData.h"
#include "datasources/HTTPTileDataSource.h"
#include "layers/Layer.h"
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
#include <boost/lexical_cast.hpp>

#include <picojson/picojson.h>

namespace carto {

    CartoMapsService::CartoMapsService() :
        _username(),
        _apiKey(),
        _apiTemplate(DEFAULT_API_TEMPLATE),
        _tilerURL(),
        _statTag(),
        _layerFilter(),
        _authTokens(),
        _layerIndices(),
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

    std::string CartoMapsService::getTilerURL() const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        return _tilerURL;
    }

    void CartoMapsService::setTilerURL(const std::string& tilerURL) {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        _tilerURL = tilerURL;
    }

    std::string CartoMapsService::getStatTag() const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        return _statTag;
    }

    void CartoMapsService::setStatTag(const std::string& statTag) {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        _statTag = statTag;
    }

    std::string CartoMapsService::getLayerFilter() const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        return _layerFilter;
    }

    void CartoMapsService::setLayerFilter(const std::string& filter) {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        _layerFilter = filter;
    }

    std::vector<std::string> CartoMapsService::getAuthTokens() const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        return _authTokens;
    }

    void CartoMapsService::setAuthTokens(const std::vector<std::string>& authTokens) {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        _authTokens = authTokens;
    }

    std::vector<int> CartoMapsService::getLayerIndices() const {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        return _layerIndices;
    }

    void CartoMapsService::setLayerIndices(const std::vector<int>& layerIndices) {
        std::lock_guard<std::recursive_mutex> lock(*_mutex);
        _layerIndices = layerIndices;
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

    void CartoMapsService::buildNamedMap(std::vector<std::shared_ptr<Layer> >& layers, const std::string& templateId, const std::map<std::string, Variant>& templateParams) const {
        // TODO: global catch/log
        std::lock_guard<std::recursive_mutex> lock(*_mutex);

        // Build URL
        std::string url = getAPITemplate(picojson::value(picojson::object()));
        url += "/api/v1/map/named/" + NetworkUtils::URLEncode(templateId);
        url = getServiceURL(url);

        // Create request data by serializing parameters
        std::string paramsJSON = Variant(templateParams).toString();
        auto requestData = std::make_shared<BinaryData>(reinterpret_cast<const unsigned char*>(paramsJSON.data()), paramsJSON.size());

        // Perform HTTP request
        HTTPClient client(false);
        std::shared_ptr<BinaryData> responseData;
        std::map<std::string, std::string> responseHeaders;
        if (client.post(url, "application/json", requestData, std::map<std::string, std::string>(), responseHeaders, responseData) != 0) {
            std::string result;
            if (responseData) {
                result = std::string(reinterpret_cast<const char*>(responseData->data()), responseData->size());
            }
            Log::Errorf("CartoMapsService::buildNamedMap: Failed to read map configuration: %s", result.c_str());
            return;
        }
        
        // Read layergroupid, meta data
        std::string result(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        picojson::value mapInfo;
        std::string err = picojson::parse(mapInfo, result);
        if (!err.empty()) {
            Log::Errorf("CartoMapsService::buildNamedMap: Failed to parse response: %s", err.c_str());
            return;
        }

        std::string layerGroupId = mapInfo.get("layergroupid").get<std::string>();
        const picojson::value& metadata = mapInfo.get("metadata");
        if (metadata.is<picojson::object>()) {
            // TODO: option to group raster/vector layers
            const picojson::array& layerConfigs = metadata.get("layers").get<picojson::array>();
            for (auto it = layerConfigs.begin(); it != layerConfigs.end(); it++) {
                const picojson::value& layerConfig = *it;
                std::string type = boost::algorithm::to_lower_copy(layerConfig.get("type").get<std::string>());
                const picojson::value& options = layerConfig.get("meta");

                // If layer filter is defined, check the type of the layer first
                if (!_layerFilter.empty()) {
                    std::string paddedLayerFilter = "," + _layerFilter + ",";
                    if (paddedLayerFilter.find("," + type + ",") == std::string::npos) {
                        // TODO: what to do if filter is 'mapnik' but layer is 'cartodb'?
                        continue;
                    }
                }

                int layerIndex = it - layerConfigs.begin();
                if (!_layerIndices.empty() && std::find(_layerIndices.begin(), _layerIndices.end(), layerIndex) == _layerIndices.end()) {
                    continue;
                }
                createLayer(layers, picojson::value(picojson::object()), type == "torque" ? "torque" : "mapnik", options, layerGroupId, std::vector<int> { layerIndex });
            }
        } else {
            createLayer(layers, picojson::value(picojson::object()), "mapnik", picojson::value(picojson::object()), layerGroupId, _layerIndices);
        }
    }

    void CartoMapsService::buildMap(std::vector<std::shared_ptr<Layer> >& layers, const Variant& mapConfig) const {
        // TODO: global catch/log
        std::lock_guard<std::recursive_mutex> lock(*_mutex);

        // Build URL
        std::string url = getAPITemplate(mapConfig.toPicoJSON());
        url += "/api/v1/map";
        url = getServiceURL(url);

        // Create request data
        std::string mapConfigJSON = mapConfig.toString();
        auto requestData = std::make_shared<BinaryData>(reinterpret_cast<const unsigned char*>(mapConfigJSON.data()), mapConfigJSON.size());

        // Do HTTP POST request
        HTTPClient client(false);
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
// TODO: read cartocss from mapInfo?
            const picojson::value& layerConfig = *it;
            std::string type = boost::algorithm::to_lower_copy(layerConfig.get("type").get<std::string>());
            const picojson::value& options = layerConfig.get("options");

            // If layer filter is defined, check the type of the layer first
            if (!_layerFilter.empty()) {
                std::string paddedLayerFilter = "," + _layerFilter + ",";
                if (paddedLayerFilter.find("," + type + ",") == std::string::npos) {
                    continue;
                }
            }

            createLayer(layers, mapConfig.toPicoJSON(), type, options, layerGroupId, std::vector<int> { });
        }
    }

    int CartoMapsService::getMinZoom(const picojson::value& options) const {
        if (options.contains("minzoom")) {
            return static_cast<int>(options.get("minzoom").get<std::int64_t>());
        }
        if (options.contains("minZoom")) {
            return static_cast<int>(options.get("minZoom").get<std::int64_t>());
        }
        return 0;
    }

    int CartoMapsService::getMaxZoom(const picojson::value& options) const {
        if (options.contains("maxzoom")) {
            return static_cast<int>(options.get("maxzoom").get<std::int64_t>());
        }
        if (options.contains("maxZoom")) {
            return static_cast<int>(options.get("maxZoom").get<std::int64_t>());
        }
        return Const::MAX_SUPPORTED_ZOOM_LEVEL;
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

    std::string CartoMapsService::getTilerURL(const picojson::value& mapConfig) const {
        if (!_tilerURL.empty()) {
            std::string username = getUsername(mapConfig);
            std::map<std::string, std::string> tagValues = { { "user", username }, { "username", username } };
            return GeneralUtils::ReplaceTags(_apiTemplate, tagValues, "{", "}", false);
        }
        return getAPITemplate(mapConfig);
    }

    std::string CartoMapsService::getServiceURL(const std::string& baseURL) const {
        std::string url = baseURL;

        std::multimap<std::string, std::string> urlParams;
        if (!_apiKey.empty()) {
            urlParams.insert({ "api_key", _apiKey });
            url = NetworkUtils::SetURLProtocol(url, "https");
        }

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

    void CartoMapsService::createLayer(std::vector<std::shared_ptr<Layer> >& layers, const picojson::value& mapConfig, const std::string& type, const picojson::value& options, const std::string& layerGroupId, const std::vector<int>& layerIndices) const {
        if (type == "plain" || type == "background") {
            // TODO: if layers empty and plain color, set background color
            // Otherwise create RasterTileLayer with constant-color tiles?
            // Warn if imageUrl is defined
            Log::Warnf("CartoMapsService::createLayer: unimplemented layer type: %s", type.c_str());
        }
        else if (type == "mapnik" || type == "carto" || type == "cartodb" || type == "torque") {
            // TODO: use cdn parameter?
            std::string urlTemplateBase = getTilerURL(mapConfig);
            urlTemplateBase += "/api/v1/map/" + layerGroupId;
            for (std::size_t i = 0; i < layerIndices.size(); i++) {
                urlTemplateBase += (i == 0 ? "/" : ",") + boost::lexical_cast<std::string>(layerIndices[i]);
            }
            if (layerIndices.empty() && options.contains("interactivity")) {
                urlTemplateBase += "/0"; // TODO: hack
            }
            urlTemplateBase += "/{z}/{x}/{y}";

            std::size_t layerIndex = layers.size();

            LayerType layerType = _defaultLayerType;
            auto it = _layerTypes.find(layers.size());
            if (it != _layerTypes.end()) {
                layerType = it->second;
            }

            std::string cartoCSS;
            if (options.contains("cartocss") && options.get("cartocss").is<std::string>()) {
                cartoCSS = options.get("cartocss").get<std::string>();
            }

            if (type == "torque") {
                auto dataSource = std::make_shared<HTTPTileDataSource>(getMinZoom(mapConfig), getMaxZoom(mapConfig), urlTemplateBase + ".torque");
                // TODO: AssetPackage support
                auto styleSet = std::make_shared<CartoCSSStyleSet>(cartoCSS);
                auto torqueTileDecoder = std::make_shared<TorqueTileDecoder>(styleSet);
                auto layer = std::make_shared<TorqueTileLayer>(dataSource, torqueTileDecoder);
                layers.push_back(layer);
            }
            else if (layerType == LAYER_TYPE_VECTOR && !cartoCSS.empty()) {
                auto dataSource = std::make_shared<HTTPTileDataSource>(getMinZoom(mapConfig), getMaxZoom(mapConfig), urlTemplateBase + ".mvt");
                // TODO: AssetPackage support
                auto styleSet = std::make_shared<CartoCSSStyleSet>(cartoCSS);
                auto vectorTileDecoder = std::make_shared<MBVectorTileDecoder>(styleSet);
                auto layer = std::make_shared<VectorTileLayer>(dataSource, vectorTileDecoder);
                layers.push_back(layer);
            }
            else {
                auto dataSource = std::make_shared<HTTPTileDataSource>(getMinZoom(mapConfig), getMaxZoom(mapConfig), urlTemplateBase + ".png");
                auto layer = std::make_shared<RasterTileLayer>(dataSource);
                layers.push_back(layer);
            }

            if (options.contains("interactivity")) {
                auto dataSource = std::make_shared<HTTPTileDataSource>(getMinZoom(mapConfig), getMaxZoom(mapConfig), urlTemplateBase + ".grid.json");
                for (std::size_t i = layerIndex; i < layers.size(); i++) {
                    std::static_pointer_cast<TileLayer>(layers[i])->setUTFGridDataSource(dataSource);
                }
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
            layers.push_back(layer);
        }
        else if (type == "named" || type == "namedmap") {
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

            CartoMapsService mapsService(*this);
            mapsService.setAuthTokens(authTokens);
            mapsService.buildNamedMap(layers, name, params);
        }
        else if (type == "layergroup") {
            buildMap(layers, Variant::FromPicoJSON(options));
        }
        else {
            Log::Warnf("CartoMapsService::createLayer: Unsupported layer type: %s", type.c_str());
        }
    }

    const std::string CartoMapsService::DEFAULT_API_TEMPLATE = "http://{user}.cartodb.com";

}
