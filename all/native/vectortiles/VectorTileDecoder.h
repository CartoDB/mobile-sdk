/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORTILEDECODER_H_
#define _CARTO_VECTORTILEDECODER_H_

#include "graphics/Color.h"

#include <memory>
#include <string>
#include <mutex>
#include <map>
#include <vector>
#include <tuple>

#include <cglib/mat.h>

namespace carto {
    namespace vt {
        struct TileId;
        class Tile;
        struct BitmapPattern;
    }

    class BinaryData;
    class Feature;
    class MapBounds;

    /**
     * Abstract base class for vector tile decoders.
     */
    class VectorTileDecoder {
    public:
        typedef std::map<int, std::shared_ptr<const vt::Tile> > TileMap;
        typedef std::tuple<long long, std::string, std::shared_ptr<Feature> > TileFeature;

        /**
         * Interface for monitoring decoder parameter change events.
         */
        struct OnChangeListener {
            virtual ~OnChangeListener() { }
    
            /**
             * Listener method that gets called when decoder parameters have changed and need to be updated.
             */
            virtual void onDecoderChanged() = 0;
        };
    
        virtual ~VectorTileDecoder();
    
        /**
         * Returns background color for tiles.
         * @return Background color for tiles.
         */
        virtual Color getBackgroundColor() const = 0;
    
        /**
         * Returns background pattern image for tiles.
         * @return background pattern image for tiles.
         */
        virtual std::shared_ptr<const vt::BitmapPattern> getBackgroundPattern() const = 0;

        /**
         * Returns minimum zoom level supported for by the decoder (or style).
         * @return Minimum supported zoom level.
         */
        virtual int getMinZoom() const = 0;
    
        /**
         * Returns maximum zoom level supported for by the decoder (or style).
         * @return Maximum supported zoom level.
         */
        virtual int getMaxZoom() const = 0;

        /**
         * Decoders the specified feature from the tile layer.
         * @param id The id of the feature to decode.
         * @param tile The tile coordinates.
         * @param tileData The tile data to use.
         * @param tileBounds The bounds for the tile (used for coordinate transformation).
         * @return The feature, if found. Null if not found.
         */
        virtual std::shared_ptr<TileFeature> decodeFeature(long long id, const vt::TileId& tile, const std::shared_ptr<BinaryData>& tileData, const MapBounds& tileBounds) const = 0;
        
        /**
         * Loads the specified vector tile.
         * @param tile The id of the tile to load.
         * @param targetTile The target tile id that will be created from the data.
         * @param tileData The tile data to decode.
         * @return The vector tile data, for each frame. If the tile is not available, null is returned.
         */
        virtual std::shared_ptr<TileMap> decodeTile(const vt::TileId& tile, const vt::TileId& targetTile, const std::shared_ptr<BinaryData>& tileData) const = 0;
    
        /**
         * Notifies listeners that the decoder parameters have changed. Action taken depends on the implementation of the
         * listeners, but generally all cached tiles will be reloaded. 
         */
        virtual void notifyDecoderChanged();
        
        /**
         * Registers listener for decoder change events.
         * @param listener The listener for change events.
         */
        void registerOnChangeListener(const std::shared_ptr<OnChangeListener>& listener);
    
        /**
         * Unregisters listener from decoder change events.
         * @param listener The previously added listener.
         */
        void unregisterOnChangeListener(const std::shared_ptr<OnChangeListener>& listener);
        
    protected:
        VectorTileDecoder();

        static cglib::mat3x3<float> calculateTileTransform(const carto::vt::TileId& tileId, const carto::vt::TileId& targetTileId);
        
    private:
        std::vector<std::shared_ptr<OnChangeListener> > _onChangeListeners;
        mutable std::mutex _onChangeListenersMutex;
    };
        
}

#endif
