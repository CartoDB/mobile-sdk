/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_HILLSHADERASTERTILELAYER_H_
#define _CARTO_HILLSHADERASTERTILELAYER_H_

#include "graphics/Color.h"
#include "components/DirectorPtr.h"
#include "layers/RasterTileLayer.h"
#include "rastertiles/ElevationDecoder.h"

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
        explicit HillshadeRasterTileLayer(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<ElevationDecoder>& elevationDecoder);
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

        /**
         * Returns the shadow color of the layer.
         * @return The shadow color of the layer.
         */
        Color getShadowColor() const;
        /**
         * Sets the shadow color of the layer.
         * @param color The new shadow color of the layer.
         */
        void setShadowColor(const Color& color);

        /**
         * Returns the highlight color of the layer.
         * @return The highlight color of the layer.
         */
        Color getHighlightColor() const;
        /**
         * Sets the highlight color of the layer.
         * @param color The new highlight color of the layer.
         */
        void setHighlightColor(const Color& color);
        /**
         * Returns the illumination direction of the layer.
         * @return direction in degrees.
         */
        float getIlluminationDirection() const;
        /**
         * Sets the illumination direction.
         * @param direction in degrees.
         */
        void setIlluminationDirection(float direction);
        /**
         * Returns wheter the illumination direction should change with the map rotation.
         * @return enabled
         */
        bool getIlluminationMapRotationEnabled() const;
        /**
         * Sets wheter the illumination direction should change with the map rotation.
         * @param enabled whether to enable or not.
         */
        void setIlluminationMapRotationEnabled(bool enabled);

        double getElevation(const MapPos& pos) const;
        std::vector<double> getElevations(const std::vector<MapPos> poses) const;

    protected:
        virtual bool onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState);

        virtual std::shared_ptr<vt::Tile> createVectorTile(const MapTile& tile, const std::shared_ptr<Bitmap>& bitmap) const;

        std::shared_ptr<Bitmap> getMapTileBitmap(const MapTile& mapTile) const;

        const DirectorPtr<ElevationDecoder> _elevationDecoder;
   
        float _contrast;
        float _heightScale;
        Color _shadowColor;
        Color _highlightColor;
        float _illuminationDirection;
        bool _illuminationMapRotationEnabled;
    };
    
}

#endif
