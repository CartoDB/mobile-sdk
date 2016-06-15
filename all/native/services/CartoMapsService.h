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

        bool isInteractive() const;
        void setInteractive(bool interactive);

        std::vector<int> getLayerIndices() const;
        void setLayerIndices(const std::vector<int>& layerIndices);

        std::vector<std::string> getLayerFilter() const;
        void setLayerFilter(const std::vector<std::string>& filter);

        std::vector<std::string> getAuthTokens() const;
        void setAuthTokens(const std::vector<std::string>& authTokens);

        std::map<std::string, std::string> getCDNURLs() const;
        void setCDNURLs(const std::map<std::string, std::string>& cdnURLs);

        bool isDefaultVectorLayerMode() const;
        void setDefaultVectorLayerMode(bool enabled);

        std::shared_ptr<AssetPackage> getVectorTileAssetPackage() const;
        void setVectorTileAssetPackage(const std::shared_ptr<AssetPackage>& assetPackage);

        std::vector<std::shared_ptr<Layer> > buildMap(const Variant& mapConfig) const;

        std::vector<std::shared_ptr<Layer> > buildNamedMap(const std::string& templateId, const std::map<std::string, Variant>& templateParams) const;

    private:
        std::string getServiceURL(const std::string& path) const;

        std::string getTilerURL(const std::map<std::string, std::string>& cdnURLs) const;

        std::shared_ptr<Layer> createLayer(int layerIndex, const std::string& layerGroupId, const std::string& type, const std::string& cartoCSS, const std::map<std::string, std::string>& cdnURLs) const;

        std::vector<std::shared_ptr<Layer> > createLayers(const picojson::value& mapInfo) const;

        static const std::string DEFAULT_API_TEMPLATE;
        
        std::string _username;
        std::string _apiKey;
        std::string _apiTemplate;
        std::string _tilerURL;
        std::string _statTag;
        bool _interactive;
        std::vector<int> _layerIndices;
        std::vector<std::string> _layerFilter;
        std::vector<std::string> _authTokens;
        std::map<std::string, std::string> _cdnURLs;
        bool _defaultVectorLayerMode;
        std::shared_ptr<AssetPackage> _vectorTileAssetPackage;

        mutable std::recursive_mutex _mutex;
    };

}

#endif
