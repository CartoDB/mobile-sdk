#include "CartoVisLoader.h"
#include "core/BinaryData.h"
#include "layers/Layer.h"
#include "layers/Layers.h"
#include "network/HTTPClient.h"
#include "services/CartoMapsService.h"
#include "services/CartoUIBuilder.h"
#include "ui/BaseMapView.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <algorithm>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <picojson/picojson.h>

namespace carto {

    CartoVisLoader::CartoVisLoader() :
        _cartoUIBuilder(),
        _cartoUIBuilderMutex()
    {
    }

    CartoVisLoader::~CartoVisLoader() {
    }

    std::shared_ptr<CartoUIBuilder> CartoVisLoader::getCartoUIBuilder() const {
        std::lock_guard<std::mutex> lock(_cartoUIBuilderMutex);
        return _cartoUIBuilder.get();
    }

    void CartoVisLoader::setCartoUIBuilder(const std::shared_ptr<CartoUIBuilder>& cartoUIBuilder) {
        std::lock_guard<std::mutex> lock(_cartoUIBuilderMutex);
        _cartoUIBuilder = DirectorPtr<CartoUIBuilder>(cartoUIBuilder);
    }

    void CartoVisLoader::loadVis(const std::shared_ptr<BaseMapView>& mapView, const std::string& visURL) const {
        HTTPClient client(false);
        std::shared_ptr<BinaryData> responseData;
        std::map<std::string, std::string> responseHeaders;
        if (client.get(visURL, std::map<std::string, std::string>(), responseHeaders, responseData) != 0) {
            // TODO:
            Log::Error("CartoVisLoader::loadVis: Failed to read VisJSON configuration");
            return;
        }

        // Read VisJSON
        std::string result(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        picojson::value visJSON;
        std::string err = picojson::parse(visJSON, result);
        if (!err.empty()) {
            Log::Errorf("CartoVisLoader::loadVis: Failed to parse VisJSON configuration: %s", err.c_str());
            return;
        }

        // TODO: zoom, bounds, etc

        std::vector<std::shared_ptr<Layer> > layers;
        if (visJSON.contains("layers")) {
            const picojson::array& layerConfigs = visJSON.get("layers").get<picojson::array>();
            for (auto it = layerConfigs.begin(); it != layerConfigs.end(); it++) {
                createLayer(layers, *it);
            }
        }
        std::for_each(layers.begin(), layers.end(), [&mapView](const std::shared_ptr<Layer>& layer) {
            mapView->getLayers()->add(layer);
        });
    }

    void CartoVisLoader::configureMapsService(CartoMapsService& mapsService, const picojson::value& options) const {
        if (options.contains("user_name")) {
            mapsService.setUsername(options.get("user_name").get<std::string>());
        }

        if (options.contains("maps_api_template")) {
            mapsService.setAPITemplate(options.get("maps_api_template").get<std::string>());
        }

        if (options.contains("stat_tag")) {
            mapsService.setStatTag(options.get("stat_tag").get<std::string>());
        }

        if (options.contains("filter")) {
            mapsService.setLayerFilter(options.get("filter").get<std::string>());
        }

        std::string tilerProtocol = options.contains("tiler_protocol") ? options.get("tiler_protocol").get<std::string>() : "http";
        std::string tilerDomain = options.contains("tiler_domain") ? options.get("tiler_domain").get<std::string>() : "cartodb.com";
        std::string tilerPort = options.contains("tiler_port") ? boost::lexical_cast<std::string>(options.get("tiler_port").get<std::int64_t>()) : (tilerProtocol == "https" ? "443" : "80");
        mapsService.setTilerURL(tilerProtocol + "://" + (options.contains("user_name") ? "{user}." : "") + tilerDomain + ":" + tilerPort);
    }

    void CartoVisLoader::createLayer(std::vector<std::shared_ptr<Layer> >& layers, const picojson::value& layerConfig) const {
        std::string type = boost::algorithm::to_lower_copy(layerConfig.get("type").get<std::string>());
        const picojson::value& options = layerConfig.get("options");

        std::size_t layerIndex = layers.size();

        if (type == "tiled" || type == "plain" || type == "background") {
            CartoMapsService mapsService;
            configureMapsService(mapsService, options);
            mapsService.buildMap(layers, Variant::FromPicoJSON(options));
        }
        else if (type == "torque") {
            if (options.contains("named_map")) {
                const picojson::value& namedMap = options.get("named_map");

                std::string name = namedMap.get("name").get<std::string>();
                std::map<std::string, Variant> params;
                const picojson::object& paramConfig = namedMap.get("params").get<picojson::object>();
                for (auto it = paramConfig.begin(); it != paramConfig.end(); it++) {
                    params[it->first] = Variant::FromPicoJSON(it->second);
                }
                int layerIndex = static_cast<int>(namedMap.get("layer_index").get<std::int64_t>());

                CartoMapsService mapsService;
                configureMapsService(mapsService, options);
                mapsService.setLayerIndices(std::vector<int>{ layerIndex });
                mapsService.buildNamedMap(layers, name, params);
            }
            else {
                std::map<std::string, Variant> layerConfig;
                layerConfig["table_name"] = Variant(options.get("table_name").get<std::string>());
                layerConfig["sql"] = Variant(options.contains("query") ? options.get("query").get<std::string>() : options.get("sql").get<std::string>());
                layerConfig["cartocss"] = Variant(options.contains("tile_style") ? options.get("tile_style").get<std::string>() : options.get("cartocss").get<std::string>());
                layerConfig["cartocss_version"] = Variant(options.contains("cartocss_version") ? options.get("cartocss_version").get<std::string>() : "2.1.1");
                std::map<std::string, Variant> mapConfig;
                mapConfig["type"] = Variant(type);
                mapConfig["options"] = Variant(mapConfig);

                CartoMapsService mapsService;
                configureMapsService(mapsService, options);
                mapsService.buildMap(layers, Variant(mapConfig));
            }
        }
        else if (type == "namedmap") {
            const picojson::value& namedMap = options.get("named_map");

            std::string name = namedMap.get("name").get<std::string>();
            std::map<std::string, Variant> params;
            const picojson::object& paramConfig = namedMap.get("params").get<picojson::object>();
            for (auto it = paramConfig.begin(); it != paramConfig.end(); it++) {
                params[it->first] = Variant::FromPicoJSON(it->second);
            }

            CartoMapsService mapsService;
            configureMapsService(mapsService, options);
            mapsService.buildNamedMap(layers, name, params);

            if (namedMap.contains("layers")) {
                const picojson::array& layerConfigs = namedMap.get("layers").get<picojson::array>();
                for (auto it = layerConfigs.begin(); it != layerConfigs.end(); it++) {
                    const picojson::value& layerConfig = *it;
                    if (layerConfig.contains("infowindow")) {
                        const picojson::value& infoWindow = layerConfig.get("infowindow");
                        // TODO: add listener to layer
                    }
                }
            }
        }
        else if (type == "layergroup") {
            const picojson::value& layerDefinition = options.get("layer_definition");

            CartoMapsService mapsService;
            configureMapsService(mapsService, options);
            mapsService.buildMap(layers, Variant::FromPicoJSON(layerDefinition));

            if (layerDefinition.contains("layers")) {
                const picojson::array& layerConfigs = layerDefinition.get("layers").get<picojson::array>();
                for (auto it = layerConfigs.begin(); it != layerConfigs.end(); it++) {
                    const picojson::value& layerConfig = *it;
                    if (layerConfig.contains("infowindow")) {
                        const picojson::value& infoWindow = layerConfig.get("infowindow");
                        // TODO: add listener to layer
                    }
                }
            }
        }
        else {
            // unsupported
            Log::Warnf("Unsupported layer type: %s", type.c_str()); // TODO:
        }

        // Update general layer attributes
        for (std::size_t i = layerIndex; i < layers.size(); i++) {
            if (layerConfig.contains("visible")) {
                layers[i]->setVisible(layerConfig.get("visible").get<bool>());
            }
        }
    }

}
