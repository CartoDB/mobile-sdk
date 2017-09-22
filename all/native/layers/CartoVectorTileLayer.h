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
             * The Positron style. Replacement for older 'grey' style.
             */
            CARTO_BASEMAP_STYLE_POSITRON,
            /**
             * The Darkmatter style. Replacement for older 'dark' style.
             */
            CARTO_BASEMAP_STYLE_DARKMATTER,
            /**
             * The Voyager style. Replacement for older 'bright' style.
             */
            CARTO_BASEMAP_STYLE_VOYAGER
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
         * Constructs a CartoVectorTileLayer object from a source name and a style asset package.
         * Style asset package defines visual style of the map and must be compatible with the source.
         * @param dataSource The data source from which this layer loads data.
         * @param styleAssetPackage The style asset package (usually a zipped file or an asset)
         */
        CartoVectorTileLayer(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<AssetPackage>& styleAssetPackage);
        /**
         * Constructs a CartoVectorTileLayer object from a source name and a style asset package.
         * Style asset package defines visual style of the map and must be compatible with the source.
         * @param dataSource The data source from which this layer loads data.
         * @param styleAssetPackage The style asset package (usually a zipped file or an asset)
         * @param styleName The style to use from the asset package.
         */
        CartoVectorTileLayer(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<AssetPackage>& styleAssetPackage, const std::string& styleName);
        virtual ~CartoVectorTileLayer();

        /**
         * Returns the current language used for the layer.
         * @return The current language. If the returned string is empty, then 'local' languages are used.
         */
        std::string getLanguage() const;
        /**
         * Sets the current map language.
         * @param lang The new language to use. The default is local language (empty string).
         */
        void setLanguage(const std::string& lang);

        /**
         * Returns the current fallback language used for the layer. Fallback language is used when a primary language name is not available.
         * @return The current fallback language. If the returned string is empty, then 'local' languages are used.
         */
        std::string getFallbackLanguage() const;
        /**
         * Sets the current fallback map language. Fallback language is used when a primary language name is not available.
         * @param lang The new fallback language to use. The default is local language (empty string).
         */
        void setFallbackLanguage(const std::string& lang);

        /**
         * Creates a new tile decoder from the specified base map style.
         * @param style The style to use for the decoder.
         * @return The new vector tile decoder configured for the style.
         */
        static std::shared_ptr<VectorTileDecoder> CreateTileDecoder(CartoBaseMapStyle::CartoBaseMapStyle style);
        /**
         * Creates a new tile decoder from the specified asset package.
         * @param styleAssetPackage The style asset package (usually a zipped file or an asset)
         * @return The new vector tile decoder configured for the style.
         */
        static std::shared_ptr<VectorTileDecoder> CreateTileDecoder(const std::shared_ptr<AssetPackage>& styleAssetPackage);
        /**
         * Creates a new tile decoder from the specified asset package.
         * @param styleAssetPackage The style asset package (usually a zipped file or an asset)
         * @param styleName The name of the style to use.
         * @return The new vector tile decoder configured for the style.
         */
        static std::shared_ptr<VectorTileDecoder> CreateTileDecoder(const std::shared_ptr<AssetPackage>& styleAssetPackage, const std::string& styleName);

        static std::shared_ptr<AssetPackage> CreateStyleAssetPackage();

        static std::string GetStyleName(CartoBaseMapStyle::CartoBaseMapStyle style);
    
        static std::string GetStyleSource(CartoBaseMapStyle::CartoBaseMapStyle style);
    };
    
}

#endif
