/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ELEVATIONDECODER_H_
#define _CARTO_ELEVATIONDECODER_H_

#include "graphics/Color.h"
#include "core/MapTile.h"
#include "core/MapPos.h"

#include <memory>
#include <string>
#include <mutex>
#include <map>
#include <vector>

#include <cglib/mat.h>

#include <mapnikvt/Map.h>

namespace carto {
    class Bitmap;
    class TileDataSource;
    /**
     * Abstract base class for raster elevation decoders.
     */
    class ElevationDecoder {
    public:
    /**
         * Constructs an ElevationDecoder.
         */
        ElevationDecoder();
        virtual ~ElevationDecoder();
        
        virtual float decodeHeight(const Color& encodedHeight) const;
        virtual std::array<float, 4> getVectorTileScales() const = 0;
        virtual std::array<float, 4> getColorComponentCoefficients() const = 0;

        int getElevation(std::shared_ptr<TileDataSource> dataSource, const MapPos &pos) const;
        std::vector<int> getElevations(std::shared_ptr<TileDataSource> dataSource, const std::vector<MapPos> poses) const;

    protected:
        std::shared_ptr<Bitmap> getMapTileBitmap(std::shared_ptr<TileDataSource> dataSource, const MapTile& mapTile) const;

    };
        
}

#endif
