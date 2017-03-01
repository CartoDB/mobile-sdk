/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOVISLOADER_H_
#define _CARTO_CARTOVISLOADER_H_

#include "core/Variant.h"
#include "components/DirectorPtr.h"

#include <memory>
#include <mutex>
#include <string>

#include <boost/optional.hpp>

#include <picojson/picojson.h>

namespace carto {
    class AssetPackage;
    class Layer;
    class BaseMapView;
    class CartoMapsService;
    class CartoVisBuilder;

    /**
     * A high level interface for loading VisJSON configurations.
     * VisJSON defines full map visualizations, together with
     * base maps and interactive overlays.
     * Caution! The class is highly EXPERIMENTAL and only works with limited
     * number of v2 visualizations and with even fewer v3 visualizations!
     */
    class CartoVisLoader {
    public:
        /**
         * Constructs a new instance of CartoVisLoader with default settings.
         */
        CartoVisLoader();
        virtual ~CartoVisLoader();

        /**
         * Returns true if the service configures layers as vector tile layers,
         * when possible. By default this is false. Vector layers
         * provide much better visual quality at the expense of performance.
         * @return True if vector layers should be used. False is raster layers are used.
         */
        bool isDefaultVectorLayerMode() const;
        /**
         * Sets the service to vector layer mode or raster layer mode.
         * By default service creates raster layers, but vector layers may
         * provide much better visual quality at the expense of performance.
         * @param vectorLayerMode True if vector layers should be used. False is raster layers are used.
         */
        void setDefaultVectorLayerMode(bool vectorLayerMode);

        /**
         * Returns true if 'strict mode' is enabled (the service throws exceptions whenever an error is encountered).
         * @return True if 'strict mode' is enabled.
         */
        bool isStrictMode() const;
        /**
         * Sets the 'strict mode' flag.
         * @param strictMode True if all errors result in exceptions. The default is false.
         */
        void setStrictMode(bool strictMode);

        /**
         * Returns the asset package used when decoding vector tiles.
         * By default, no asset package is used and null is returned.
         * @return The asset package used when decoding vector tiles.
         */
        std::shared_ptr<AssetPackage> getVectorTileAssetPackage() const;
        /**
         * Sets the asset package used when decoding vector tiles.
         * Asset package with fonts is needed when texts are used.
         * @param assetPackage The asset package to use for vector tiles.
         */
        void setVectorTileAssetPackage(const std::shared_ptr<AssetPackage>& assetPackage);

        /**
         * Loads a specified online VisJSON configuration.
         * The actual map configuration must be done via callbacks in
         * the specified CartoVisBuilder instance.
         * @param builder The handler that receives events for configuring the map.
         * @param visURL The VisJSON URL to use.
         * @throws std::runtime_error If IO error occured during the operation.
         */
        void loadVis(const std::shared_ptr<CartoVisBuilder>& builder, const std::string& visURL) const;

    private:
        struct LayerInfo {
            std::shared_ptr<Layer> layer;
            picojson::object attributes;

            LayerInfo(const std::shared_ptr<Layer>& layer, const picojson::object& attributes) : layer(layer), attributes(attributes) { }
        };

        static void readLayerAttributes(picojson::object& attributes, const picojson::value& options);
        
        void configureMapsService(CartoMapsService& mapsService, const picojson::value& options) const;
        
        void configureLayerInteractivity(Layer& layer, const picojson::value& options) const;

        void createLayers(const std::shared_ptr<CartoVisBuilder>& builder, const picojson::value& layerConfig) const;

        boost::optional<LayerInfo> createTiledLayer(const picojson::value& options) const;
        boost::optional<LayerInfo> createBackgroundLayer(const picojson::value& options) const;
        boost::optional<LayerInfo> createCartoLayer(const std::string& type, const picojson::value& options, const picojson::value& legend) const;
        std::vector<LayerInfo> createNamedLayers(const picojson::value& options) const;
        std::vector<LayerInfo> createLayerGroup(const picojson::value& options, const picojson::value& infoWindow) const;
        std::vector<LayerInfo> createLayerInfos(const std::vector<std::shared_ptr<Layer> >& layers, const picojson::value& options, const picojson::value& layerDefinition) const;

        static const std::string DEFAULT_TILER_PROTOCOL;
        static const std::string DEFAULT_TILER_DOMAIN;

        bool _defaultVectorLayerMode;
        bool _strictMode;
        std::shared_ptr<AssetPackage> _vectorTileAssetPackage;

        mutable std::string _defaultUsername;

        mutable std::recursive_mutex _mutex;
    };

}

#endif
