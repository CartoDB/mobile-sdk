/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOVECTORTILEDECODER_H_
#define _CARTO_CARTOVECTORTILEDECODER_H_

#include "vectortiles/VectorTileDecoder.h"

#include <memory>
#include <mutex>
#include <set>
#include <map>
#include <vector>
#include <string>

#include <mapnikvt/Value.h>

namespace carto {
    namespace mvt {
        class Map;
        class MBVTFeatureDecoder;
        class SymbolizerContext;
        class Logger;
    }

    class AssetPackage;
    class CartoCSSStyleSet;
    
    /**
     * Decoder for Carto vector tiles in MapBox format.
     */
    class CartoVectorTileDecoder : public VectorTileDecoder {
    public:
        /**
         * Constructs a decoder for Carto vector tiles based on specified CartoCSS style set.
         * @param layerIds The list of layer ids.
         * @param layerStyleSets The CartoCSS style sets for the layers.
         * @throws std::runtime_error If the decoder could not be created or there are issues with the style set.
         */
        explicit CartoVectorTileDecoder(const std::vector<std::string>& layerIds, const std::map<std::string, std::shared_ptr<CartoCSSStyleSet> >& layerStyleSets);
        virtual ~CartoVectorTileDecoder();

        /**
         * Returns the list of layer ids used when decoding a tile.
         * @return The list of layer ids used.
         */
        std::vector<std::string> getLayerIds() const;

        /**
         * Returns the visibility of the specified layer.
         * @param layerId The layer name to use.
         * @return True if the layer is visible, false otherwise.
         */
        bool isLayerVisible(const std::string& layerId) const;
        /**
         * Sets the visibility of the specified layer.
         * @param layerId The layer name to use.
         * @param visible True if the layer should be visible, false if hidden.
         */
        void setLayerVisible(const std::string& layerId, bool visible);

        /**
         * Returns the CartoCSS style set used by the decoder for the specified layer.
         * @param layerId The layer name to use.
         * @return The given layer CartoCSS style.
         * @throws std::out_of_range If the layer id is not valid.
         */
        std::shared_ptr<CartoCSSStyleSet> getLayerStyleSet(const std::string& layerId) const;
        /**
         * Sets the CartoCSS style set used by the decoder for the specified layer.
         * @param layerId The layer name to use.
         * @param styleSet The new style set to use.
         * @throws std::runtime_error If the decoder could not be updated or there are issues with the CartoCSS.
         * @throws std::out_of_range If the layer id is not valid.
         */
        void setLayerStyleSet(const std::string& layerId, const std::shared_ptr<CartoCSSStyleSet>& styleSet);

        virtual Color getBackgroundColor() const;
    
        virtual std::shared_ptr<const vt::BitmapPattern> getBackgroundPattern() const;
        
        virtual int getMinZoom() const;
        
        virtual int getMaxZoom() const;

        virtual std::shared_ptr<TileFeature> decodeFeature(long long id, const vt::TileId& tile, const std::shared_ptr<BinaryData>& tileData, const MapBounds& tileBounds) const;

        virtual std::shared_ptr<TileMap> decodeTile(const vt::TileId& tile, const vt::TileId& targetTile, const std::shared_ptr<BinaryData>& tileData) const;
    
    protected:
        void updateLayerStyleSet(const std::string& layerId, const std::shared_ptr<CartoCSSStyleSet>& styleSet);

        static const int DEFAULT_TILE_SIZE;
        static const int STROKEMAP_SIZE;
        static const int GLYPHMAP_SIZE;
        
        const std::shared_ptr<mvt::Logger> _logger;
        const std::vector<std::string> _layerIds;
        std::set<std::string> _layerInvisibleSet;
        std::map<std::string, std::shared_ptr<CartoCSSStyleSet> > _layerStyleSets;
        std::map<std::string, std::shared_ptr<mvt::Map> > _layerMaps;
        std::map<std::string, std::shared_ptr<mvt::SymbolizerContext> > _layerSymbolizerContexts;
        std::map<std::shared_ptr<AssetPackage>, std::shared_ptr<mvt::SymbolizerContext> > _assetPackageSymbolizerContexts;
        Color _backgroundColor;
        std::shared_ptr<const vt::BitmapPattern> _backgroundPattern;

        mutable std::pair<std::shared_ptr<BinaryData>, std::shared_ptr<mvt::MBVTFeatureDecoder> > _cachedFeatureDecoder;
    
        mutable std::mutex _mutex;
    };
        
}

#endif
