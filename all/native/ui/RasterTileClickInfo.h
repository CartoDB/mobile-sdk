/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_RASTERTILECLICKINFO_H_
#define _CARTO_RASTERTILECLICKINFO_H_

#include "core/MapPos.h"
#include "core/MapTile.h"
#include "graphics/Color.h"
#include "ui/ClickType.h"

#include <memory>
#include <string>

namespace carto {
    class Layer;
    
    /**
     * A container class that provides information about a click performed on raster tile.
     */
    class RasterTileClickInfo {
    public:
        /**
         * Constructs a RasterTileClickInfo object from a click position, tile information and clicked feature.
         * @param clickType The click type (SINGLE, DUAL, etc)
         * @param clickPos The click position in the coordinate system of the base projection.
         * @param mapTile The clicked tile id.
         * @param nearestColor The color of the nearest pixel to the click position.
         * @param interpolatedColor The interpolated color at the click position.
         * @param layer The layer of the raster tile on which the click was performed.
         */
        RasterTileClickInfo(ClickType::ClickType clickType, const MapPos& clickPos, const MapTile& mapTile, const Color& nearestColor, const Color& interpolatedColor, const std::shared_ptr<Layer>& layer);
        virtual ~RasterTileClickInfo();
    
        /**
         * Returns the click type.
         * @return The type of the click performed.
         */
        ClickType::ClickType getClickType() const;

        /**
         * Returns the click position.
         * @return The click position in the coordinate system of the base projection.
         */
        const MapPos& getClickPos() const;
        
        /**
         * Returns the tile id of the clicked feature.
         * @return The tile id of the clicked feature.
         */
        const MapTile& getMapTile() const;

        /**
         * Returns the color of the nearest pixel to the click position.
         * @return The color of the nearest pixel to the click position.
         */
        const Color& getNearestColor() const;

        /**
         * Returns the interpolated color at the click position.
         * @return The interpolated color at the click position.
         */
        const Color& getInterpolatedColor() const;

        /**
         * Returns the layer of the raster tile.
         * @return The layer of the raster tile.
         */
        std::shared_ptr<Layer> getLayer() const;
    
    private:
        ClickType::ClickType _clickType;
        MapPos _clickPos;
        MapTile _mapTile;
    
        Color _nearestColor;
        Color _interpolatedColor;
        std::shared_ptr<Layer> _layer;
    };
    
}

#endif
