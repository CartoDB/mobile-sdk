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
    
    class TileUtils {
    public:
        static MapTile CalculateMapTile(const MapPos& mapPos, int zoom, const std::shared_ptr<Projection>& proj);

        static MapPos CalculateMapTileOrigin(const MapTile& mapTile, const std::shared_ptr<Projection>& proj);

        static MapBounds CalculateMapTileBounds(const MapTile& mapTile, const std::shared_ptr<Projection>& proj);

    private:
        TileUtils();
    };
    
}

#endif
