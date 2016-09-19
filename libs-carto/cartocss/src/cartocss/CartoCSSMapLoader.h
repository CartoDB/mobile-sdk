/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSS_CARTOCSSMAPLOADER_H_
#define _CARTO_CARTOCSS_CARTOCSSMAPLOADER_H_

#include "CartoCSSCompiler.h"
#include "CartoCSSMapnikTranslator.h"
#include "Value.h"
#include "mapnikvt/Map.h"
#include "mapnikvt/Logger.h"

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace carto { namespace css {
    class CartoCSSMapLoader {
    public:
        struct LoaderException : std::runtime_error {
        public:
            explicit LoaderException(const std::string& msg) : runtime_error(msg) { }
        };

        class AssetLoader {
        public:
            virtual ~AssetLoader() = default;

            virtual std::shared_ptr<std::vector<unsigned char>> load(const std::string& fileName) const = 0;
        };

        explicit CartoCSSMapLoader(std::shared_ptr<AssetLoader> assetLoader, std::shared_ptr<mvt::Logger> logger) : _assetLoader(std::move(assetLoader)), _logger(std::move(logger)) { }
        virtual ~CartoCSSMapLoader() = default;

        void setIgnoreLayerPredicates(bool ignore) { _ignoreLayerPredicates = ignore; }

        std::shared_ptr<mvt::Map> loadMap(const std::string& cartoCSS) const;

        std::shared_ptr<mvt::Map> loadMapProject(const std::string& fileName) const;

    protected:
        constexpr static int MAX_ZOOM = 24;

        struct AttachmentStyle {
            std::string attachment;
            int order = 0;
            float opacity = 1.0f;
            std::string compOp;
            std::vector<std::shared_ptr<const mvt::Rule>> rules;
        };
        
        template <typename T>
        static bool getMapProperty(const std::map<std::string, Value>& mapProperties, const std::string& name, T& value) {
            auto valueIt = mapProperties.find(name);
            if (valueIt != mapProperties.end()) {
                if (auto valuePtr = boost::get<T>(&valueIt->second)) {
                    value = *valuePtr;
                    return true;
                }
            }
            return false;
        }

        std::shared_ptr<mvt::Map> buildMap(const StyleSheet& styleSheet, const std::vector<std::string>& layerNames, const std::vector<mvt::NutiParameter>& nutiParameters) const;
        void loadMapSettings(const std::map<std::string, Value>& mapProperties, mvt::Map::Settings& mapSettings) const;
        void buildAttachmentStyleMap(const CartoCSSMapnikTranslator& translator, const std::shared_ptr<mvt::Map>& map, int zoom, const std::list<CartoCSSCompiler::LayerAttachment>& layerAttachments, std::map<std::string, AttachmentStyle>& attachmentStyleMap) const;
        std::vector<AttachmentStyle> getSortedAttachmentStyles(const std::map<std::string, AttachmentStyle>& attachmentStyleMap) const;

        bool _ignoreLayerPredicates = false;
        std::shared_ptr<AssetLoader> _assetLoader;
        std::shared_ptr<mvt::Logger> _logger;
    };
} }

#endif
