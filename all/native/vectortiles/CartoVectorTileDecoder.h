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
    
    /**
     * Decoder for Carto vector tiles in MapBox format.
     */
    class CartoVectorTileDecoder : public VectorTileDecoder {
    public:
        /**
         * Constructs a decoder for Carto vector tiles based on specified CartoCSS style set.
         * @param layerIds The list of layer ids.
         * @param layerStyles The CartoCSS style sets for the layer.
         * @param assetPackage The asset package to use. Can be null.
         * @throws std::runtime_error If the decoder could not be created or there are issues with the style set.
         */
        explicit CartoVectorTileDecoder(const std::vector<std::string>& layerIds, const std::map<std::string, std::string>& layerStyles, const std::shared_ptr<AssetPackage>& assetPackage);
        virtual ~CartoVectorTileDecoder();

        /**
         * Returns the list of layer ids used when decoding a tile.
         * @return The list of layer ids used.
         */
        std::vector<std::string> getLayerIds() const;

        /**
         * Returns the current CartoCSS used by the decoder for the specified layer.
         * @param layerId The layer index to use.
         * @return The given layer CartoCSS style.
         * @throws std::out_of_range If the layer id is not valid.
         */
        std::string getLayerStyle(const std::string& layerId) const;
        /**
         * Sets the current CartoCSS style set used by the decoder for the specified layer.
         * @param layerId The layer name to use.
         * @param cartoCSS The new style set to use.
         * @throws std::runtime_error If the decoder could not be updated or there are issues with the CartoCSS.
         * @throws std::out_of_range If the layer id is not valid.
         */
        void setLayerStyle(const std::string& layerId, const std::string& cartoCSS);

        virtual Color getBackgroundColor() const;
    
        virtual std::shared_ptr<const vt::BitmapPattern> getBackgroundPattern() const;
        
        virtual int getMinZoom() const;
        
        virtual int getMaxZoom() const;

        virtual std::shared_ptr<TileFeature> decodeFeature(long long id, const vt::TileId& tile, const std::shared_ptr<BinaryData>& tileData, const MapBounds& tileBounds) const;

        virtual std::shared_ptr<TileMap> decodeTile(const vt::TileId& tile, const vt::TileId& targetTile, const std::shared_ptr<BinaryData>& tileData) const;
    
    protected:
        void updateLayerStyle(const std::string& layerId, const std::string& cartoCSS);

        static const int DEFAULT_TILE_SIZE;
        static const int STROKEMAP_SIZE;
        static const int GLYPHMAP_SIZE;
        
        const std::shared_ptr<mvt::Logger> _logger;
        const std::shared_ptr<AssetPackage> _assetPackage;
        const std::vector<std::string> _layerIds;
        std::map<std::string, std::string> _layerStyles;
        std::map<std::string, std::shared_ptr<mvt::Map> > _layerMaps;
        Color _backgroundColor;
        std::shared_ptr<const vt::BitmapPattern> _backgroundPattern;
        std::shared_ptr<mvt::SymbolizerContext> _symbolizerContext;

        mutable std::pair<std::shared_ptr<BinaryData>, std::shared_ptr<mvt::MBVTFeatureDecoder> > _cachedFeatureDecoder;
    
        mutable std::mutex _mutex;
    };
        
}

#endif
