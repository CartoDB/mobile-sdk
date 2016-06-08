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
    class Layer;

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

        std::string getTilerURL() const;
        void setTilerURL(const std::string& tilerURL);

        std::string getStatTag() const;
        void setStatTag(const std::string& statTag);

        std::string getLayerFilter() const;
        void setLayerFilter(const std::string& filter);

        std::vector<std::string> getAuthTokens() const;
        void setAuthTokens(const std::vector<std::string>& authTokens);

        std::vector<int> getLayerIndices() const;
        void setLayerIndices(const std::vector<int>& layerIndices);

        bool isDefaultVectorLayerMode() const;
        void setDefaultVectorLayerMode(bool enabled);

        bool isVectorLayerMode(int index) const;
        void setVectorLayerMode(int index, bool enabled);

        void buildNamedMap(std::vector<std::shared_ptr<Layer> >& layers, const std::string& templateId, const std::map<std::string, Variant>& templateParams) const;
        void buildMap(std::vector<std::shared_ptr<Layer> >& layers, const Variant& mapConfig) const;

    private:
        enum LayerType {
            LAYER_TYPE_RASTER,
            LAYER_TYPE_VECTOR
        };
        
        int getMinZoom(const picojson::value& options) const;
        int getMaxZoom(const picojson::value& options) const;

        std::string getUsername(const picojson::value& mapConfig) const;
        std::string getAPITemplate(const picojson::value& mapConfig) const;
        std::string getTilerURL(const picojson::value& mapConfig) const;

        std::string getServiceURL(const std::string& baseURL) const;

        void createLayer(std::vector<std::shared_ptr<Layer> >& layers, const picojson::value& mapConfig, const picojson::value& layerConfig, const std::string& layerGroupId) const;

        static std::string escapeParam(const std::string& value, const std::string& type);

        static void replaceTemplateTags(picojson::value& value, const std::map<std::string, std::string>& tagValues);
        static void replaceTemplateParams(picojson::value& templateObject, const picojson::object& placeholders, const std::map<std::string, Variant>& templateParams);

        static const std::string DEFAULT_API_TEMPLATE;
        
        std::string _username;
        std::string _apiKey;
        std::string _apiTemplate;
        std::string _tilerURL;
        std::string _statTag;
        std::string _layerFilter;
        std::vector<std::string> _authTokens;
        std::vector<int> _layerIndices;
        LayerType _defaultLayerType;
        std::map<int, LayerType> _layerTypes;

        mutable std::shared_ptr<std::recursive_mutex> _mutex;
    };

}

#endif
