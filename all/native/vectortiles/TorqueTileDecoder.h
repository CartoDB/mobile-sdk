/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TORQUETILEDECODER_H_
#define _CARTO_TORQUETILEDECODER_H_

#include "vectortiles/VectorTileDecoder.h"

#include <memory>
#include <mutex>
#include <map>
#include <string>

#include <mapnikvt/Value.h>

namespace carto {
    namespace mvt {
        class TorqueMap;
        class SymbolizerContext;
        class Logger;
    }

    class CartoCSSStyleSet;

    /**
     * A decoder for Torque layer that accepts json-based Torque tiles.
     */
    class TorqueTileDecoder : public VectorTileDecoder {
    public:
        /**
         * Constructs a new TorqueTileDecoder given style.
         * @param styleSet The style set used by decoder.
         * @throws std::runtime_error If the decoder could not be created or there are issues with the style set.
         */
        explicit TorqueTileDecoder(const std::shared_ptr<CartoCSSStyleSet>& styleSet);
        virtual ~TorqueTileDecoder();

        /**
         * Returns the frame count defined in the Torque style.
         * @return The frame count in the animation.
         */
        int getFrameCount() const;

        /**
         * Returns the current style set used by the decoder.
         * @return The current style set.
         */
        std::shared_ptr<CartoCSSStyleSet> getStyleSet() const;
        /**
         * Sets the current style set used by the decoder.
         * @param styleSet The new style set to use.
         * @throws std::runtime_error If the decoder could not be updated or there are issues with the style set.
         */
        void setStyleSet(const std::shared_ptr<CartoCSSStyleSet>& styleSet);

        /**
         * Returns the tile resolution, in pixels. Default is 256.
         * @return The tile resolution in pixels.
         */
        int getResolution() const;
        /**
         * Sets the tile resolution in pixels. Default is 256.
         * @param resolution The new resolution value.
         */
        void setResolution(int resolution);

        virtual Color getBackgroundColor() const;

        virtual std::shared_ptr<const vt::BitmapPattern> getBackgroundPattern() const;

        virtual int getMinZoom() const;

        virtual int getMaxZoom() const;

        virtual std::shared_ptr<TileFeature> decodeFeature(long long id, const vt::TileId& tile, const std::shared_ptr<BinaryData>& tileData, const MapBounds& tileBounds) const;

        virtual std::shared_ptr<TileMap> decodeTile(const vt::TileId& tile, const vt::TileId& targetTile, const std::shared_ptr<BinaryData>& tileData) const;

    protected:
        void updateCurrentStyle(const std::shared_ptr<CartoCSSStyleSet>& styleSet);

        static const int DEFAULT_TILE_SIZE;
        static const int GLYPHMAP_SIZE;

        const std::shared_ptr<mvt::Logger> _logger;
        int _resolution;
        std::shared_ptr<mvt::TorqueMap> _map;
        std::shared_ptr<mvt::SymbolizerContext> _symbolizerContext;
        std::shared_ptr<CartoCSSStyleSet> _styleSet;

        mutable std::mutex _mutex;
    };

}

#endif
