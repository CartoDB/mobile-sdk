/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GDALRASTERTILEDATASOURCE_H_
#define _CARTO_GDALRASTERTILEDATASOURCE_H_

#ifdef _CARTO_GDAL_SUPPORT

#include "core/MapBounds.h"
#include "datasources/TileDataSource.h"

#include <cglib/vec.h>
#include <cglib/mat.h>

class GDALDataset;
class OGRSpatialReference;

namespace carto {
    class Projection;

    /**
     * High-level raster tile data source that supports various GDAL data formats.
     * For example, GeoTiff files can be used using this data source.
     */
    class GDALRasterTileDataSource : public TileDataSource {
    public:
        /**
         * Constructs a new GDAL raster data source.
         * @param minZoom The minimum zoom level supported by this data source.
         * @param maxZoom The maximum zoom level supported by this data source.
         * @param fileName The full path of the data file
         * @throws std::exception If the file could not be opened.
         */
        GDALRasterTileDataSource(int minZoom, int maxZoom, const std::string& fileName);
        /**
         * Constructs a new GDAL raster data source with explicit reference to SRS.
         * @param minZoom The minimum zoom level supported by this data source.
         * @param maxZoom The maximum zoom level supported by this data source.
         * @param fileName The full path of the data file
         * @param srs The spatial reference system of the data ("EPSG:34326", for example)
         * @throws std::exception If the file could not be opened.
         */
        GDALRasterTileDataSource(int minZoom, int maxZoom, const std::string& fileName, const std::string& srs);
        virtual ~GDALRasterTileDataSource();

        /**
         * Returns the extent of this data source. Extent is the minimal bounding box encompassing all the raster data.
         * @return The minimal bounding box for the data.
         */
        MapBounds getDataExtent() const;

        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);
        
    private:
        void initializeTransform(const std::shared_ptr<OGRSpatialReference>& poDatasetSpatialRef);

        GDALDataset* _poDataset;
        int _width;
        int _height;
        int _tileSize;
        bool _hasAlpha;
        cglib::mat3x3<double> _transform;
        cglib::mat3x3<double> _invTransform;
        std::shared_ptr<Projection> _projection;

        mutable std::mutex _mutex;

        static const float FILTER_SCALE;
        static const int MAX_FILTER_WIDTH;
        static const int MAX_DOWNSAMPLE_FACTOR;
    };
}

#endif

#endif
