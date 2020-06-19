/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_HILLSHADERASTERTILELAYER_H_
#define _CARTO_HILLSHADERASTERTILELAYER_H_

#include "layers/RasterTileLayer.h"

namespace carto {
    
    /**
     * A tile layer that displays an overlay hillshading. Should be used together with corresponding data source that encodes height in RGBA image.
     * The shading is based on the direction of the main light source, which can be configured using Options class.
     * Note: this class is experimental and may change or even be removed in future SDK versions.
     */
    class HillshadeRasterTileLayer : public RasterTileLayer {
    public:
        /**
         * Constructs a HillshadeRasterTileLayer object from a data source.
         * @param dataSource The data source from which this layer loads data.
         */
        explicit HillshadeRasterTileLayer(const std::shared_ptr<TileDataSource>& dataSource);
        virtual ~HillshadeRasterTileLayer();

        /**
         * Returns the contrast of the hillshade overlay.
         * @return The contrast value (between 0..1). Default is 0.5.
         */
        float getContrast() const;
        /**
         * Sets the contrast of the hillshade overlay.
         * @param contrast The contrast value (between 0..1).
         */
        void setContrast(float contrast);

        /**
         * Returns the height scale of the hillshade overlay.
         * @return The relative height scale. Default is 1.0.
         */
        float getHeightScale() const;
        /**
         * Sets the height scale of the hillshade overlay.
         * @param heightScale The relative height scale. Actual height is multiplied by this values.
         */
        void setHeightScale(float heightScale);
    
    protected:
        virtual std::shared_ptr<vt::Tile> createVectorTile(const MapTile& tile, const std::shared_ptr<Bitmap>& bitmap) const;

        float _contrast;
        float _heightScale;
    };
    
}

#endif
