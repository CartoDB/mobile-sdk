/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOMAPSSERVICE_H_
#define _CARTO_CARTOMAPSSERVICE_H_

#include "core/Variant.h"

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <map>

#include <picojson/picojson.h>

namespace carto {
    class AssetPackage;
    class Layer;

    /**
     * A high-level interface for Carto Maps Service.
     * The service can be used to automatically configure layers using
     * anonymous map configurations or by using parametrized named maps.
     */
    class CartoMapsService {
    public:
        /**
         * Constructs a new CartoMapsService instance with default settings.
         */
        CartoMapsService();
        virtual ~CartoMapsService();
        
        /**
         * Returns the user account name used when accessing the online service.
         * @return The user account name used when accessing the online service.
         */
        std::string getUsername() const;
        /**
         * Sets the user account name used when accessing the online service.
         * @param username The user account name used when accessing the online service.
         */
        void setUsername(const std::string& username);

        /**
         * Returns the API endpoint template of the online service.
         * @return The API endpoint template of the online service.
         */
        std::string getAPITemplate() const;
        /**
         * Sets the API endpoint template of the online service.
         * By default cartodb.com endpoint is used. This needs to be redefined only for on-premise services.
         * @param apiTemplate The API endpoint template of the online service. For example, "http://{user}.cartodb.com"
         */
        void setAPITemplate(const std::string& apiTemplate);

        /**
         * Returns the tiler URL used by the online service.
         * @return The tiler URL used by the online service.
         */
        std::string getTilerURL() const;
        /**
         * Sets the tiler URL used by the online service.
         * By default, API template is used as the tiler URL.
         * @param tilerURL The tiler URL used by the online service.
         */
        void setTilerURL(const std::string& tilerURL);

        /**
         * Returns the 'stat tag' parameter passed to the online service.
         * @return The 'stat tag' parameter passed to the online service.
         */
        std::string getStatTag() const;
        /**
         * Sets the 'stat tag' parameter passed to the online service.
         * @param statTag The 'stat tag' parameter passed to the online service. The default is empty.
         */
        void setStatTag(const std::string& statTag);

        /**
         * Returns true is the service configures
         * layers as interactive by using UTF grids for click detection.
         * @return True if the service configures layers as interactive.
         */
        bool isInteractive() const;
        /**
         * Sets the interactivity flag of the layers.
         * @param interactive True if layers should be configured as interactive. The default is false.
         */
        void setInteractive(bool interactive);

        /**
         * Returns the list of layer indices that are created.
         * If non-empty, only layers from this list are created.
         * @return The list of layer indices that are created.
         */
        std::vector<int> getLayerIndices() const;
        /**
         * Sets the list of layer indices that are created.
         * If non-empty, only layers from this list are created.
         * @param layerIndices The list of layer indices that are created.
         */
        void setLayerIndices(const std::vector<int>& layerIndices);

        /**
         * Returns the layer filter, i.e. the list of layer types that are created.
         * If non-empty, only layer types from this list are created.
         * @return The list of layer types that are created.
         */
        std::vector<std::string> getLayerFilter() const;
        /**
         * Sets the layer filter, i.e. the list of layer types that are created.
         * If non-empty, only layer types from this list are created.
         * @param filter The list of layer types that are created.
         */
        void setLayerFilter(const std::vector<std::string>& filter);

        /**
         * Returns the list of authentication tokens used when accessing online service.
         * @return The list of authentication tokens used when accessing online service.
         */
        std::vector<std::string> getAuthTokens() const;
        /**
         * Sets the list of authentication tokens used when accessing online service.
         * @param authTokens The list of authentication tokens used when accessing online service.
         */
        void setAuthTokens(const std::vector<std::string>& authTokens);

        /**
         * Returns the map of protocols and CDN URLs used when accessing online service.
         * @return The map of protocols and CDN URLs used when accessing online service.
         */
        std::map<std::string, std::string> getCDNURLs() const;
        /**
         * Sets the map of protocols and CDN URLs used when accessing online service.
         * @param cdnURLs The map of protocols and CDN URLs used when accessing online service.
         */
        void setCDNURLs(const std::map<std::string, std::string>& cdnURLs);

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
         * Builds a list of layers given anonymous map configuration.
         * The map configuration specification can be found in CartoDB documentation page.
         * @param mapConfig The map configuration JSON deserialized as Variant type.
         * @return The list of created layers.
         * @throws std::runtime_error If IO error occured during the operation.
         */
        std::vector<std::shared_ptr<Layer> > buildMap(const Variant& mapConfig) const;

        /**
         * Builds a list of layers given named map id and named map parameters.
         * @param templateId The named map id.
         * @param templateParams The map containing named map parameter names and their values.
         * @return The list of created layers.
         * @throws std::runtime_error If IO error occured during the operation.
         */
        std::vector<std::shared_ptr<Layer> > buildNamedMap(const std::string& templateId, const std::map<std::string, Variant>& templateParams) const;

    private:
        struct LayerInfo {
            int index;
            std::string id;
            std::string cartoCSS;

            LayerInfo(int index, const std::string& id, const std::string& cartoCSS) : index(index), id(id), cartoCSS(cartoCSS) { }
        };

        std::string getServiceURL(const std::string& path) const;

        std::string getTilerURL(const std::map<std::string, std::string>& cdnURLs) const;

        std::shared_ptr<Layer> createLayerGroup(const std::string& layerGroupId, const std::string& type, const std::vector<LayerInfo>& layerInfos, const std::map<std::string, std::string>& cdnURLs) const;

        std::vector<std::shared_ptr<Layer> > createLayers(const picojson::value& mapInfo) const;

        static const std::string DEFAULT_API_TEMPLATE;
        
        std::string _username;
        std::string _apiTemplate;
        std::string _tilerURL;
        std::string _statTag;
        bool _interactive;
        std::vector<int> _layerIndices;
        std::vector<std::string> _layerFilter;
        std::vector<std::string> _authTokens;
        std::map<std::string, std::string> _cdnURLs;
        bool _defaultVectorLayerMode;
        bool _strictMode;
        std::shared_ptr<AssetPackage> _vectorTileAssetPackage;

        mutable std::recursive_mutex _mutex;
    };

}

#endif
