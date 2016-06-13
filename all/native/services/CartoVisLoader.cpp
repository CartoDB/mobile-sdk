#include "CartoVisLoader.h"
#include "core/BinaryData.h"
#include "datasources/HTTPTileDataSource.h"
#include "layers/Layer.h"
#include "layers/Layers.h"
#include "layers/RasterTileLayer.h"
#include "network/HTTPClient.h"
#include "services/CartoMapsService.h"
#include "services/CartoVisBuilder.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <algorithm>
#include <regex>
#include <unordered_map>
#include <vector>

#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <picojson/picojson.h>

namespace {
    
    boost::optional<bool> getBool(const picojson::value& val) {
        if (val.is<bool>()) {
            return val.get<bool>();
        }
        if (val.is<std::int64_t>()) {
            return val.get<std::int64_t>() != 0;
        }
        if (val.is<double>()) {
            return val.get<double>() != 0;
        }
        if (val.is<std::string>()) {
            if (val.get<std::string>() == "true") {
                return true;
            }
            if (val.get<std::string>() == "false") {
                return true;
            }
            return boost::lexical_cast<bool>(val.get<std::string>());
        }
        return boost::optional<bool>();
    }

    boost::optional<int> getInt(const picojson::value& val) {
        if (val.is<bool>()) {
            return val.get<bool>() ? 1 : 0;
        }
        if (val.is<std::int64_t>()) {
            return static_cast<int>(val.get<std::int64_t>());
        }
        if (val.is<double>()) {
            return static_cast<int>(val.get<double>());
        }
        if (val.is<std::string>()) {
            return boost::lexical_cast<int>(val.get<std::string>());
        }
        return boost::optional<int>();
    }

    boost::optional<double> getDouble(const picojson::value& val) {
        if (val.is<bool>()) {
            return val.get<bool>() ? 1 : 0;
        }
        if (val.is<std::int64_t>()) {
            return static_cast<double>(val.get<std::int64_t>());
        }
        if (val.is<double>()) {
            return val.get<double>();
        }
        if (val.is<std::string>()) {
            return boost::lexical_cast<double>(val.get<std::string>());
        }
        return boost::optional<double>();
    }

    boost::optional<std::string> getString(const picojson::value& val) {
        if (val.is<bool>()) {
            return val.get<bool>() ? "true" : "false";
        }
        if (val.is<std::int64_t>()) {
            return boost::lexical_cast<std::string>(val.get<std::int64_t>());
        }
        if (val.is<double>()) {
            return boost::lexical_cast<std::string>(val.get<double>());
        }
        if (val.is<std::string>()) {
            return val.get<std::string>();
        }
        return boost::optional<std::string>();
    }

    boost::optional<carto::MapPos> getMapPos(const picojson::value& val) {
        if (val.is<picojson::array>()) {
            const picojson::array& arr = val.get<picojson::array>();
            if (arr.size() == 2) {
                if (auto x = getDouble(arr[0])) {
                    if (auto y = getDouble(arr[1])) {
                        return carto::MapPos(*x, *y);
                    }
                }
            }
        }
        return boost::optional<carto::MapPos>();
    }

    boost::optional<carto::MapBounds> getMapBounds(const picojson::value& val) {
        if (val.is<picojson::array>()) {
            const picojson::array& arr = val.get<picojson::array>();
            if (arr.size() == 2) {
                if (auto min = getMapPos(arr[0])) {
                    if (auto max = getMapPos(arr[1])) {
                        return carto::MapBounds(*min, *max);
                    }
                }
            }
        }
        return boost::optional<carto::MapBounds>();
    }

}

namespace carto {

    CartoVisLoader::CartoVisLoader() {
    }

    CartoVisLoader::~CartoVisLoader() {
    }

    bool CartoVisLoader::loadVis(const std::shared_ptr<CartoVisBuilder>& builder, const std::string& visURL) const {
        // TODO: global try/catch logger
        HTTPClient client(false);
        std::shared_ptr<BinaryData> responseData;
        std::map<std::string, std::string> responseHeaders;
        if (client.get(visURL, std::map<std::string, std::string>(), responseHeaders, responseData) != 0) {
            Log::Error("CartoVisLoader::loadVis: Failed to read VisJSON configuration");
            return false;
        }

        // Read VisJSON
        std::string result(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        picojson::value visJSON;
        std::string err = picojson::parse(visJSON, result);
        if (!err.empty()) {
            Log::Errorf("CartoVisLoader::loadVis: Failed to parse VisJSON configuration: %s", err.c_str());
            return false;
        }

        // Base options
        if (auto center = getMapPos(visJSON.get("center"))) {
            builder->setCenter(*center);
        }

        if (auto zoom = getDouble(visJSON.get("zoom"))) {
            builder->setZoom(static_cast<float>(*zoom));
        }

        if (auto bounds = getMapBounds(visJSON.get("bounds"))) {
            builder->setBounds(*bounds);
        }

        builder->setDescription(Variant::FromPicoJSON(visJSON));

        // Configue layers
        const picojson::value& layersOption = visJSON.get("layers");
        if (layersOption.is<picojson::array>()) {
            const picojson::array& layerConfigs = layersOption.get<picojson::array>();

            // Build layer orders and sort
            std::vector<std::pair<std::size_t, int> > layerOrders;
            for (const picojson::value& layerConfig : layerConfigs) {
                int order = static_cast<int>(layerOrders.size());
                if (auto orderOpt = getInt(layerConfig.get("order"))) {
                    order = *orderOpt;
                }
                layerOrders.emplace_back(layerOrders.size(), order);
            }

            std::sort(layerOrders.begin(), layerOrders.end(), [](std::pair<std::size_t, int> order1, std::pair<std::size_t, int> order2) {
                return order1.second > order2.second;
            });

            // Create layers
            for (std::size_t i = 0; i < layerOrders.size(); i++) {
                createLayers(builder, layerConfigs[layerOrders[i].first]);
            }
        }

        return true;
    }

    void CartoVisLoader::readLayerAttributes(picojson::object& attributes, const picojson::value& options) {
        if (auto name = getString(options.get("name"))) {
            attributes["name"] = picojson::value(*name);
        }
        if (auto attribution = getString(options.get("attribution"))) {
            attributes["attribution"] = picojson::value(*attribution);
        }
    }

    void CartoVisLoader::configureMapsService(CartoMapsService& mapsService, const picojson::value& options) {
        if (auto userName = getString(options.get("user_name"))) {
            mapsService.setUsername(*userName);
        }

        if (auto mapsAPITemplate = getString(options.get("maps_api_template"))) {
            mapsService.setAPITemplate(*mapsAPITemplate);
        }

        if (auto statTag = getString(options.get("stat_tag"))) {
            mapsService.setStatTag(*statTag);
        }

        if (auto filter = getString(options.get("filter"))) {
            mapsService.setLayerFilter(*filter);
        }

        std::string tilerProtocol = "http";
        if (auto tilerProtocolOpt = getString(options.get("tiler_protocol"))) {
            tilerProtocol = *tilerProtocolOpt;
        }
        std::string tilerDomain = "cartodb.com";
        if (auto tilerDomainOpt = getString(options.get("tiler_domain"))) {
            tilerDomain = *tilerDomainOpt;
        }
        std::string tilerPort = tilerProtocol == "https" ? "443" : "80";
        if (auto tilerPortOpt = getString(options.get("tiler_port"))) {
            tilerPort = *tilerPortOpt;
        }
        mapsService.setTilerURL(tilerProtocol + "://" + (options.contains("user_name") ? "{user}." : "") + tilerDomain + ":" + tilerPort);

        // TODO: cdn_url?
    }
    
    void CartoVisLoader::createLayers(const std::shared_ptr<CartoVisBuilder>& builder, const picojson::value& layerConfig) const {
        std::string type = boost::algorithm::to_lower_copy(layerConfig.get("type").get<std::string>());
        const picojson::value& options = layerConfig.get("options");

        std::vector<LayerInfo> layerInfos;
        if (type == "tiled") {
            if (auto layerInfo = createTiledLayer(options)) {
                layerInfos.push_back(*layerInfo);
            }
        }
        else if (type == "torque") {
            if (auto layerInfo = createTorqueLayer(options, layerConfig.get("legend"))) {
                layerInfos.push_back(*layerInfo);
            }
        }
        else if (type == "namedmap") {
            layerInfos = createNamedLayers(options);
        }
        else if (type == "layergroup") {
            layerInfos = createLayerGroup(options, layerConfig.get("infowindow"));
        }
        else {
            Log::Warnf("CartoVisLoader::createLayers: Unsupported layer type: %s", type.c_str());
        }

        // Final layer configuration
        for (const LayerInfo& layerInfo : layerInfos) {
            if (layerConfig.contains("visible")) {
                layerInfo.layer->setVisible(layerConfig.get("visible").get<bool>());
            }

            builder->addLayer(layerInfo.layer, Variant::FromPicoJSON(picojson::value(layerInfo.attributes)));
        }
    }

    boost::optional<CartoVisLoader::LayerInfo> CartoVisLoader::createTiledLayer(const picojson::value& options) const {
        // Create data source, based on URL template and min/max zoom
        std::string urlTemplate = *getString(options.get("urlTemplate"));
        boost::optional<int> minZoom = getInt(options.get("minZoom"));
        boost::optional<int> maxZoom = getInt(options.get("maxZoom"));
        auto dataSource = std::make_shared<HTTPTileDataSource>(minZoom ? *minZoom : 0, maxZoom ? *maxZoom : Const::MAX_SUPPORTED_ZOOM_LEVEL, urlTemplate);
        
        // Set subdomains for {s} tag
        std::vector<std::string> subdomains = { "a", "b", "c" };
        if (options.get("subdomains").is<picojson::array>()) {
            const picojson::array& subdomainsOption = options.get("subdomains").get<picojson::array>();
            subdomains.clear();
            std::transform(subdomainsOption.begin(), subdomainsOption.end(), std::back_inserter(subdomains), [](const picojson::value& subdomain) {
                return *getString(subdomain);
            });
        }
        else if (options.get("subdomains").is<std::string>()) {
            const std::string& subdomainsOption = options.get("subdomains").get<std::string>();
            subdomains.clear();
            std::transform(subdomainsOption.begin(), subdomainsOption.end(), std::back_inserter(subdomains), [](char subdomain) {
                return std::string(1, subdomain);
            });
        }
        dataSource->setSubdomains(subdomains);

        // TMS scheme flag
        auto tms = getBool(options.get("tms"));
        dataSource->setTMSScheme(tms ? *tms : false);

        // Create raster layer
        auto rasterLayer = std::make_shared<RasterTileLayer>(dataSource);
        picojson::object attributes;
        readLayerAttributes(attributes, options);
        return LayerInfo(rasterLayer, attributes);
    }

    boost::optional<CartoVisLoader::LayerInfo> CartoVisLoader::createTorqueLayer(const picojson::value& options, const picojson::value& legend) const {
        std::vector<std::shared_ptr<Layer> > layers;

        if (options.get("named_map").is<picojson::object>()) {
            // Using a layer from a named map
            const picojson::value& namedMap = options.get("named_map");

            // Read name, layer index and parameters
            std::string name = *getString(namedMap.get("name"));
            int layerIndex = 0;
            if (auto layerIndexOpt = getInt(namedMap.get("layer_index"))) {
                layerIndex = *layerIndexOpt;
            }

            std::map<std::string, Variant> params;
            const picojson::object& paramConfig = namedMap.get("params").get<picojson::object>();
            for (auto it = paramConfig.begin(); it != paramConfig.end(); it++) {
                params[it->first] = Variant::FromPicoJSON(it->second);
            }

            // Configure map service and get the layers
            CartoMapsService mapsService;
            configureMapsService(mapsService, options);
            mapsService.setLayerIndices(std::vector<int>{ layerIndex });
            layers = mapsService.buildNamedMap(name, params);
        }
        else {
            // Build map config for Maps service
            std::map<std::string, Variant> layerConfig;
            layerConfig["table_name"] = Variant(*getString(options.get("table_name")));
            if (auto query = getString(options.get("query"))) {
                layerConfig["sql"] = Variant(*query);
            }
            if (auto sql = getString(options.get("sql"))) {
                layerConfig["sql"] = Variant(*sql);
            }
            if (auto tileStyle = getString(options.get("tile_style"))) {
                layerConfig["cartocss"] = Variant(*tileStyle);
            }
            if (auto cartoCSS = getString(options.get("cartocss"))) {
                layerConfig["cartocss"] = Variant(*cartoCSS);
            }
            layerConfig["cartocss_version"] = Variant("2.1.1");
            if (auto cartoCSSVersion = getString(options.get("cartocss_version"))) {
                layerConfig["cartocss_version"] = Variant(*cartoCSSVersion);
            }

            std::map<std::string, Variant> mapConfig;
            mapConfig["type"] = Variant("torque");
            mapConfig["options"] = Variant(mapConfig);

            CartoMapsService mapsService;
            configureMapsService(mapsService, options);
            layers = mapsService.buildMap(Variant(mapConfig));
        }

        if (layers.empty()) {
            return boost::optional<LayerInfo>();
        }

        // Create attributes for the layer
        picojson::object attributes;
        readLayerAttributes(attributes, options);
        if (!legend.is<picojson::null>()) {
            attributes["legend"] = legend;
        }
        
        // Create layer info
        return LayerInfo(layers.front(), attributes);
    }

    std::vector<CartoVisLoader::LayerInfo> CartoVisLoader::createNamedLayers(const picojson::value& options) const {
        const picojson::value& namedMap = options.get("named_map");

        // Read name and parameters
        std::string name = *getString(namedMap.get("name"));
        
        std::map<std::string, Variant> params;
        const picojson::object& paramConfig = namedMap.get("params").get<picojson::object>();
        for (auto it = paramConfig.begin(); it != paramConfig.end(); it++) {
            params[it->first] = Variant::FromPicoJSON(it->second);
        }

        // Configue Maps service and get the layers
        CartoMapsService mapsService;
        configureMapsService(mapsService, options);
        if (auto statTag = getString(namedMap.get("stat_tag"))) {
            mapsService.setStatTag(*statTag);
        }
        std::vector<std::shared_ptr<Layer> > layers = mapsService.buildNamedMap(name, params);

        // Create layer attributes
        std::map<std::shared_ptr<Layer>, picojson::object> layerAttributes;
        if (namedMap.get("layers").is<picojson::array>()) {
            const picojson::array& layerConfigs = namedMap.get("layers").get<picojson::array>();
            for (std::size_t i = 0; i < layerConfigs.size() && i < layers.size(); i++) {
                const std::shared_ptr<Layer>& layer = layers[i];
                const picojson::value& layerConfig = layerConfigs[i];
                
                if (auto visible = getBool(layerConfig.get("visible"))) {
                    layer->setVisible(*visible);
                }
                
                picojson::object attributes;
                readLayerAttributes(attributes, options);
                if (layerConfig.contains("infowindow")) {
                    attributes["infowindow"] = layerConfig.get("infowindow");
                }
                if (layerConfig.contains("legend")) {
                    attributes["legend"] = layerConfig.get("legend");
                }
                if (layerConfig.contains("layer_name")) {
                    attributes["name"] = layerConfig.get("layer_name");
                }
                layerAttributes[layer] = attributes;
            }
        }

        // Create layer info list
        std::vector<LayerInfo> layerInfos;
        for (const std::shared_ptr<Layer>& layer : layers) {
            layerInfos.push_back(LayerInfo(layer, layerAttributes[layer]));
        }
        return layerInfos;
    }

    std::vector<CartoVisLoader::LayerInfo> CartoVisLoader::createLayerGroup(const picojson::value& options, const picojson::value& infoWindow) const {
        picojson::value layerDefinition = options.get("layer_definition");
        
        // Translate layer types
        picojson::array& layersOption = layerDefinition.get("layers").get<picojson::array>();
        for (picojson::value& layerOption : layersOption) {
            std::string type = boost::to_lower_copy(layerOption.get("type").get<std::string>());
            if (type == "cartodb") {
                type = "mapnik";
            }
            layerOption.get("type").get<std::string>() = type;
        }

        // Configue Maps service and get the layers
        CartoMapsService mapsService;
        configureMapsService(mapsService, options);
        std::vector<std::shared_ptr<Layer> > layers = mapsService.buildMap(Variant::FromPicoJSON(layerDefinition));

        // Create attributes for the layer
        picojson::object attributes;
        readLayerAttributes(attributes, options);
        if (!infoWindow.is<picojson::null>()) {
            attributes["infowindow"] = infoWindow;
        }

        // Create layer info list
        std::vector<LayerInfo> layerInfos;
        for (const std::shared_ptr<Layer>& layer : layers) {
            layerInfos.push_back(LayerInfo(layer, attributes));
        }
        return layerInfos;
    }

}
