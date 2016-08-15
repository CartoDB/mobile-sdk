/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BITMAPOVERLAYRASTERTILEDATASOURCE_H_
#define _CARTO_BITMAPOVERLAYRASTERTILEDATASOURCE_H_

#include "core/MapPos.h"
#include "core/MapBounds.h"
#include "core/ScreenPos.h"
#include "datasources/TileDataSource.h"

#include <cglib/mat.h>

namespace carto {
    class Bitmap;
    class Projection;

    /**
     * Tile data source that uses given bitmap with two, three or four control points define a raster overlay.
     * Note: if two points are given, conformal transformation is calculated. If three points are given, affine transformation is calculated. In case of four points, perspective transformation is used.
     */
    class BitmapOverlayRasterTileDataSource : public TileDataSource {
    public:
        /**
         * Constructs a new bitmap overlay data source.
         * @param minZoom The minimum zoom for generated tiles.
         * @param maxZoom The maximum zoom for generated tiles.
         * @param bitmap The bitmap to use as an overlay.
         * @param projection The projection definining coordinate system of the control points.
         * @param mapPoses The geographical control points. The list must contain either 2, 3 or 4 points.
         * @param bitmapPoses The pixel coordinates in the bitmap corresponding to geographical control points. The number of coordinates must be equal to the number of control points in mapPoses list.
         * @throws std::invalid_argument If the transformation can not be calculated.
         */
        BitmapOverlayRasterTileDataSource(int minZoom, int maxZoom, const std::shared_ptr<Bitmap>& bitmap, const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& mapPoses, const std::vector<ScreenPos>& bitmapPoses);
        virtual ~BitmapOverlayRasterTileDataSource();

        /**
         * Returns the extent of this data source. Extent is the minimal bounding box encompassing all the raster data.
         * @return The minimal bounding box for the data.
         */
        MapBounds getDataExtent() const;

        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);
        
    private:
        int _tileSize;

        cglib::vec2<double> _origin;
        cglib::mat3x3<double> _transform;
        cglib::mat3x3<double> _invTransform;
        std::shared_ptr<Bitmap> _bitmap;
        std::shared_ptr<Projection> _projection;

        static const float FILTER_SCALE;
        static const int MAX_FILTER_WIDTH;
    };
}

#endif
