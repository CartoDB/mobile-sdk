/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TILEUTILS_H_
#define _CARTO_TILEUTILS_H_

#include "core/MapTile.h"
#include "core/MapPos.h"
#include "core/MapVec.h"
#include "core/MapBounds.h"

#include <memory>

namespace carto {
    class Projection;
    
    /**
     * A helper class for mapping coordinates to tiles and back.
     */
    class TileUtils {
    public:
        /**
         * Calculates the map tile corresponding to specified point and zoom level.
         * @param mapPos Coordinates for the point.
         * @param zoom Zoom level for the tile.
         * @param proj The projection to use for tile and point.
         * @return The corresponding map tile.
         */
        static MapTile CalculateMapTile(const MapPos& mapPos, int zoom, const std::shared_ptr<Projection>& proj);

        /**
         * Calculates origin coordinates for the given map tile.
         * @param mapTile The map tile to use.
         * @param proj The projection to use for origin.
         * @return The corresponding tile origin.
         */
        static MapPos CalculateMapTileOrigin(const MapTile& mapTile, const std::shared_ptr<Projection>& proj);

        /**
         * Calculates bounds for the given map tile.
         * @param mapTile The map tile to use.
         * @param proj The projection to use for bounds.
         * @return The corresponding tile bounds.
         */
        static MapBounds CalculateMapTileBounds(const MapTile& mapTile, const std::shared_ptr<Projection>& proj);

    private:
        TileUtils();
    };
    
}

#endif
