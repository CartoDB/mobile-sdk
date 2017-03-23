/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOONLINEVECTORTILELAYER_H_
#define _CARTO_CARTOONLINEVECTORTILELAYER_H_

#include "layers/CartoVectorTileLayer.h"

#include <string>
#include <memory>

namespace carto {

    /**
     * Specialized online vector tile layer that connects to Carto online tile server.
     * This layer is intended as a 'shortcut' to make using Carto online service and
     * vector tiles as simple as possible.
     */
    class CartoOnlineVectorTileLayer : public CartoVectorTileLayer {
    public:
        /**
         * Constructs a CartoOnlineVectorTileLayer object from a specificed base map style.
         * @param style The style to use for the layer.
         */
        explicit CartoOnlineVectorTileLayer(CartoBaseMapStyle::CartoBaseMapStyle style);
        /**
         * Constructs a CartoOnlineVectorTileLayer object from a source name and specified base map style.
         * @param source The tile source name. Currently only 'nutiteq' sources can be used with this constructor.
         * @param style The style to use for the layer.
         */
        CartoOnlineVectorTileLayer(const std::string& source, CartoBaseMapStyle::CartoBaseMapStyle style);
        /**
         * Constructs a CartoOnlineVectorTileLayer object from a source name and style asset package.
         * Style asset package defines visual style of the map and must be compatible with the source.
         * @param source The tile source name. The legacy source is "nutiteq.osm", new source is "mapzen.osm" but this requires different style asset package.
         * @param styleAssetPackage The style asset package (usually a zipped file or an asset)
         */
        CartoOnlineVectorTileLayer(const std::string& source, const std::shared_ptr<AssetPackage>& styleAssetPackage);
        virtual ~CartoOnlineVectorTileLayer();
    };
    
}

#endif
