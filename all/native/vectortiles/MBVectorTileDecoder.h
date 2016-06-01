/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MBVECTORTILEDECODER_H_
#define _CARTO_MBVECTORTILEDECODER_H_

#include "VectorTileDecoder.h"
#include "Mapnikvt/Value.h"

#include <memory>
#include <mutex>
#include <map>
#include <vector>
#include <string>

namespace carto {
    namespace mvt {
        class Map;
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
         */
        MBVectorTileDecoder(const std::shared_ptr<CompiledStyleSet>& compiledStyleSet);
        /**
         * Constructs a decoder for MapBox vector tiles based on specified CartoCSS style set.
         * @param cartoCSSStyleSet The CartoCSS style set for the tiles.
         */
        MBVectorTileDecoder(const std::shared_ptr<CartoCSSStyleSet>& cartoCSSStyleSet);
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
         */
        std::string getStyleParameter(const std::string& param) const;
        /**
         * Sets the value of the specified parameter.
         * The style parameter must be declared in the current style.
         * @param param The parameter to set.
         * @param value The value for the parameter.
         */
        void setStyleParameter(const std::string& param, const std::string& value);

        /**
         * Returns the value of tile buffering used. This is intended for special tile sources like MapZen.
         * @return The amount of buffering used.
         */
        float getBuffering() const;
        /**
         * Sets tile buffering factor. This is intended for special tile sources like MapZen.
         * @param buffer The amount of buffering to use. It is based on normalized tile coordinates (tile width=1.0), so 1.0/64.0 is a sensible value. The default is 0.
         */
        void setBuffering(float buffer);

        virtual Color getBackgroundColor() const;
    
        virtual std::shared_ptr<const vt::BitmapPattern> getBackgroundPattern() const;
        
        virtual int getMinZoom() const;
        
        virtual int getMaxZoom() const;
    
        virtual std::shared_ptr<TileMap> decodeTile(const vt::TileId& tile, const vt::TileId& targetTile, const std::shared_ptr<TileData>& tileData) const;
    
    protected:
        void updateCurrentStyle();

        std::shared_ptr<mvt::Map> loadMapnikMap(const std::vector<unsigned char>& styleData);
        std::shared_ptr<mvt::Map> loadCartoCSSMap(const std::string& styleAssetName, const std::shared_ptr<AssetPackage>& styleSetData);
        std::shared_ptr<mvt::Map> loadCartoCSSMap(const std::shared_ptr<CartoCSSStyleSet>& styleSet);
    
        static const int DEFAULT_TILE_SIZE;
        static const int STROKEMAP_SIZE;
        static const int GLYPHMAP_SIZE;
        
        float _buffer;
        std::shared_ptr<CompiledStyleSet> _compiledStyleSet;
        std::shared_ptr<CartoCSSStyleSet> _cartoCSSStyleSet;
        std::shared_ptr<AssetPackage> _styleSetData;
        std::shared_ptr<mvt::Map> _map;
        std::shared_ptr<mvt::Logger> _logger;
        std::shared_ptr<std::map<std::string, mvt::Value> > _parameterValueMap;
        std::shared_ptr<const vt::BitmapPattern> _backgroundPattern;
        std::shared_ptr<mvt::SymbolizerContext> _symbolizerContext;
    
        mutable std::mutex _mutex;
    };
        
}

#endif
