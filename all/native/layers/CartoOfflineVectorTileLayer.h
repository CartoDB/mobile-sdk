/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOOFFNLINEVECTORTILELAYER_H_
#define _CARTO_CARTOOFFNLINEVECTORTILELAYER_H_

#include "layers/CartoVectorTileLayer.h"

#include <string>
#include <memory>

namespace carto {
    class CartoPackageManager;

    /**
     * Specialized offline vector tile layer that uses CartoPackageManager for offline maps service.
     * This layer is intended as a 'shortcut' to make using Carto offline service and
     * vector tiles as simple as possible.
     */
    class CartoOfflineVectorTileLayer : public CartoVectorTileLayer {
    public:
        /**
         * Constructs a CartoOfflineVectorTileLayer object from a specificed base map style.
         * @param packageManager The package manager instance that handles offline maps.
         * @param style The style to use for the layer.
         */
        CartoOfflineVectorTileLayer(const std::shared_ptr<CartoPackageManager>& packageManager, CartoBaseMapStyle::CartoBaseMapStyle style);
        /**
         * Constructs a CartoOfflineVectorTileLayer object from a source name and style asset package.
         * Style asset package defines visual style of the map and must be compatible with the source.
         * @param packageManager The package manager instance that handles offline maps.
         * @param styleAssetPackage The style asset package (usually a zipped file or an asset)
         */
        CartoOfflineVectorTileLayer(const std::shared_ptr<CartoPackageManager>& packageManager, const std::shared_ptr<AssetPackage>& styleAssetPackage);
        virtual ~CartoOfflineVectorTileLayer();
    };
    
}

#endif
