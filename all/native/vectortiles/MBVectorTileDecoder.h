/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MBVECTORTILEDECODER_H_
#define _CARTO_MBVECTORTILEDECODER_H_

#include "vectortiles/VectorTileDecoder.h"

#include <memory>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <variant>

#include <mapnikvt/Value.h>

namespace carto {
    namespace mvt {
        class Map;
        class MBVTFeatureDecoder;
        class SymbolizerContext;
        class Logger;
    }

    class AssetPackage;
    class CompiledStyleSet;
    class CartoCSSStyleSet;
    
    /**
     * Decoder for vector tiles in MapBox format.
     */
    class MBVectorTileDecoder : public VectorTileDecoder {
    public:
        /**
         * Constructs a decoder for MapBox vector tiles based on specified compiled style set.
         * @param compiledStyleSet The compiled style set for the tiles.
         * @throws std::runtime_error If the decoder could not be created or there are issues with the style set.
         */
        explicit MBVectorTileDecoder(const std::shared_ptr<CompiledStyleSet>& compiledStyleSet);
        /**
         * Constructs a decoder for MapBox vector tiles based on specified CartoCSS style set.
         * @param cartoCSSStyleSet The CartoCSS style set for the tiles.
         * @throws std::runtime_error If the decoder could not be created or there are issues with the style set.
         */
        explicit MBVectorTileDecoder(const std::shared_ptr<CartoCSSStyleSet>& cartoCSSStyleSet);
        virtual ~MBVectorTileDecoder();
        
        /**
         * Returns the current compiled style set used by the decoder.
         * If decoder uses non-compiled style set, null is returned.
         * @return The current style set.
         */
        std::shared_ptr<CompiledStyleSet> getCompiledStyleSet() const;
        /**
         * Sets the current compiled style set used by the decoder.
         * @param styleSet The new style set to use.
         * @throws std::runtime_error If the decoder could not be updated or there are issues with the style set.
         */
        void setCompiledStyleSet(const std::shared_ptr<CompiledStyleSet>& styleSet);
    
        /**
         * Returns the current CartoCSS style set used by the decoder.
         * If decoder uses non-CartoCSS style set, null is returned.
         * @return The current style set.
         */
        std::shared_ptr<CartoCSSStyleSet> getCartoCSSStyleSet() const;
        /**
         * Sets the current CartoCSS style set used by the decoder.
         * @param styleSet The new style set to use.
         * @throws std::runtime_error If the decoder could not be updated or there are issues with the style set.
         */
        void setCartoCSSStyleSet(const std::shared_ptr<CartoCSSStyleSet>& styleSet);

        /**
         * Returns the list of all available style parameters.
         * @return The list of all available style parameters.
         */
        std::vector<std::string> getStyleParameters() const;
        /**
         * Returns the value of the specified style parameter.
         * The style parameter must be declared in the current style.
         * @param param The parameter to return.
         * @return The value of the parameter. If parameter does not exists, empty string is returned.
         * @throws std::invalid_argument If the style parameter does not exist.
         */
        std::string getStyleParameter(const std::string& param) const;
        /**
         * Sets the value of the specified parameter.
         * The style parameter must be declared in the current style.
         * @param param The parameter to set.
         * @param value The value for the parameter.
         * @return True if the parameter was set. False if the style parameter does not exist or could not be set.
         */
        bool setStyleParameter(const std::string& param, const std::string& value);

        /**
         * Returns the value of feature id override flag. This is intended for cases when feature ids in tile are not globally unique.
         * @return The value of feature id override flag.
         */
        bool isFeatureIdOverride() const;
        /**
         * Sets the value of feature id override flag. This is intended for cases when feature ids in tile are not globally unique.
         * @param idOverride The value of the flag.
         */
        void setFeatureIdOverride(bool idOverride);

        /**
         * Returns the value CartoCSS 'layer name ignore' flag.
         * If set to true, CSS filters like '#layer0' are ignored and the corresponding rules are applied to all filters.
         * @return The value of CartoCSS 'layer name ignore' flag. Default is false.
         */
        bool isCartoCSSLayerNamesIgnored() const;
        /**
         * Sets the value of CartoCSS 'layer name ignore' flag
         * If set to true, CSS filters like '#layer0' are ignored and the corresponding rules are applied to all filters.
         * @param ignore The value of the flag.
         */
        void setCartoCSSLayerNamesIgnored(bool ignore);

        /**
         * Returns the vector tile 'layer name override'. If empty, actual layer names are used.
         * @return The 'layer name override'.
         */
        std::string getLayerNameOverride() const;
        /**
         * Sets the 'layer name override' value. If set to non-empty value, the specific layer is used from the vector tiles.
         * @param name The new 'layer name override' value. If empty, override is not used.
         */
        void setLayerNameOverride(const std::string& name);

        virtual std::shared_ptr<const mvt::Map::Settings> getMapSettings() const;

        virtual std::shared_ptr<const mvt::SymbolizerContext::Settings> getSymbolizerContextSettings() const;

        virtual void addFallbackFont(const std::shared_ptr<BinaryData>& fontData);

        virtual int getMinZoom() const;
        
        virtual int getMaxZoom() const;

        virtual std::shared_ptr<VectorTileFeature> decodeFeature(long long id, const vt::TileId& tile, const std::shared_ptr<BinaryData>& tileData, const MapBounds& tileBounds) const;

        virtual std::shared_ptr<VectorTileFeatureCollection> decodeFeatures(const vt::TileId& tile, const std::shared_ptr<BinaryData>& tileData, const MapBounds& tileBounds) const;

        virtual std::shared_ptr<TileMap> decodeTile(const vt::TileId& tile, const vt::TileId& targetTile, const std::shared_ptr<vt::TileTransformer>& tileTransformer, const std::shared_ptr<BinaryData>& tileData) const;
    
    protected:
        void updateCurrentStyleSet(const std::variant<std::shared_ptr<CompiledStyleSet>, std::shared_ptr<CartoCSSStyleSet> >& styleSet);

        static const int DEFAULT_TILE_SIZE;
        static const int STROKEMAP_SIZE;
        static const int GLYPHMAP_SIZE;
        static const std::size_t MAX_ASSETPACKAGE_SYMBOLIZER_CONTEXTS;
        
        const std::shared_ptr<mvt::Logger> _logger;
        bool _featureIdOverride;
        bool _cartoCSSLayerNamesIgnored;
        std::string _layerNameOverride;
        std::map<std::string, mvt::Value> _parameterValueMap;
        std::vector<std::shared_ptr<BinaryData> > _fallbackFonts;
        std::variant<std::shared_ptr<CompiledStyleSet>, std::shared_ptr<CartoCSSStyleSet> > _styleSet;
        std::shared_ptr<const mvt::Map> _map;
        std::shared_ptr<const mvt::Map::Settings> _mapSettings;
        std::shared_ptr<const mvt::SymbolizerContext> _symbolizerContext;
        std::shared_ptr<const mvt::SymbolizerContext::Settings> _symbolizerContextSettings;
        std::map<std::pair<std::string, std::shared_ptr<AssetPackage> >, std::shared_ptr<const mvt::SymbolizerContext> > _assetPackageSymbolizerContexts;

        mutable std::pair<std::shared_ptr<BinaryData>, std::shared_ptr<mvt::MBVTFeatureDecoder> > _cachedFeatureDecoder;
    
        mutable std::mutex _mutex;
    };
        
}

#endif
