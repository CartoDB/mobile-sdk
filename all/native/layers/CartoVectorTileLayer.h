/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOVECTORTILELAYER_H_
#define _CARTO_CARTOVECTORTILELAYER_H_

#include "layers/VectorTileLayer.h"

#include <string>
#include <memory>

namespace carto {
    class AssetPackage;

    namespace CartoBaseMapStyle {
        /**
         * Built-in Carto basemap styles.
         */
        enum CartoBaseMapStyle {
            /**
             * The default (bright) style.
             */
            CARTO_BASEMAP_STYLE_DEFAULT,
            /**
             * The dark style.
             */
            CARTO_BASEMAP_STYLE_DARK,
            /**
             * The gray style.
             */
            CARTO_BASEMAP_STYLE_GRAY
        };
    }

    /**
     * Specialized vector tile layer that can use built-in vector tile styles.
     */
    class CartoVectorTileLayer : public VectorTileLayer {
    public:
        /**
         * Constructs a CartoVectorTileLayer object from a specificed base map style.
         * @param dataSource The data source from which this layer loads data.
         * @param style The style to use for the layer.
         */
        CartoVectorTileLayer(const std::shared_ptr<TileDataSource>& dataSource, CartoBaseMapStyle::CartoBaseMapStyle style);
        /**
         * Constructs a CartoVectorTileLayer object from a source name and style asset package.
         * Style asset package defines visual style of the map and must be compatible with the source.
         * @param dataSource The data source from which this layer loads data.
         * @param styleAssetPackage The style asset package (usually a zipped file or an asset)
         */
        CartoVectorTileLayer(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<AssetPackage>& styleAssetPackage);
        virtual ~CartoVectorTileLayer();

        /**
         * Returns the current language used for the layer.
         * @return The current language. If the returned string is empty, then 'local' languages are used.
         */
        std::string getLanguage() const;
        /**
         * Sets the current map language. The list of supported languages contains: en, de, es, it, fr, ru, zh, et.
         * @param lang The new language to use. The default is local language (empty string).
         */
        void setLanguage(const std::string& lang);
        
    private:
        static std::shared_ptr<VectorTileDecoder> CreateTileDecoder(CartoBaseMapStyle::CartoBaseMapStyle style);

        static std::shared_ptr<VectorTileDecoder> CreateTileDecoder(const std::shared_ptr<AssetPackage>& styleAssetPackage);
    };
    
}

#endif
