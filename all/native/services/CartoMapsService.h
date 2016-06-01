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
    class Layers;
    class VectorTileDecoder;
    class TorqueTileDecoder;

    class CartoMapsService {
    public:
        CartoMapsService();
        virtual ~CartoMapsService();
        
        std::string getUsername() const;
        void setUsername(const std::string& username);

        std::string getAPIKey() const;
        void setAPIKey(const std::string& apiKey);

        std::string getAPITemplate() const;
        void setAPITemplate(const std::string& apiTemplate);

        std::string getAuthToken() const;
        void setAuthToken(const std::string& authToken);

        bool isDefaultVectorLayerMode() const;
        void setDefaultVectorLayerMode(bool enabled);

        bool isVectorLayerMode(int index) const;
        void setVectorLayerMode(int index, bool enabled);

        std::shared_ptr<VectorTileDecoder> getVectorTileDecoder() const;
        void setVectorTileDecoder(const std::shared_ptr<VectorTileDecoder>& tileDecoder);

        std::shared_ptr<TorqueTileDecoder> getTorqueTileDecoder() const;
        void setTorqueTileDecoder(const std::shared_ptr<TorqueTileDecoder>& tileDecoder);

        void buildNamedMap(const std::shared_ptr<Layers>& layers, const std::string& templateId, const std::map<std::string, Variant>& templateParams) const;
        void buildMap(const std::shared_ptr<Layers>& layers, const Variant& mapConfig) const;

    private:
        enum LayerType {
            LAYER_TYPE_RASTER,
            LAYER_TYPE_VECTOR
        };
        
        int getMinZoom(const picojson::value& mapConfig) const;
        int getMaxZoom(const picojson::value& mapConfig) const;
        std::string getUsername(const picojson::value& mapConfig) const;
        std::string getAPITemplate(const picojson::value& mapConfig) const;

        void createLayer(const std::shared_ptr<Layers>& layers, const picojson::value& mapConfig, const picojson::value& layerConfig, const std::string& layerGroupId) const;

        static std::string escapeParam(const std::string& value, const std::string& type);

        static void replaceTemplateTags(picojson::value& value, const std::map<std::string, std::string>& tagValues);
        static void replaceTemplateParams(picojson::value& templateObject, const picojson::object& placeholders, const std::map<std::string, Variant>& templateParams);

        static const std::string DEFAULT_API_TEMPLATE;
        
        std::string _username;
        std::string _apiKey;
        std::string _apiTemplate;
        std::string _authToken;
        LayerType _defaultLayerType;
        std::map<int, LayerType> _layerTypes;
        std::shared_ptr<VectorTileDecoder> _vectorTileDecoder;
        std::shared_ptr<TorqueTileDecoder> _torqueTileDecoder;

        mutable std::shared_ptr<std::recursive_mutex> _mutex;
    };

}

#endif
