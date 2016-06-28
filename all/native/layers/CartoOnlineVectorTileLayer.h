/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOONLINEVECTORTILELAYER_H_
#define _CARTO_CARTOONLINEVECTORTILELAYER_H_

#include "layers/VectorTileLayer.h"

#include <string>
#include <memory>

namespace carto {
    class AssetPackage;

    /**
     * Specialized online vector tile layer that connects to Carto online tile server.
     * This layer is intended as a 'shortcut' to make using Carto online service and
     * vector tiles as simple as possible.
     */
    class CartoOnlineVectorTileLayer : public VectorTileLayer {
    public:
        /**
         * Constructs a CartoOnlineVectorTileLayer object from a source name and style asset package.
         * Style asset package defines visual style of the map and must be compatible with the source.
         * @param source The tile source name. The legacy source is "nutiteq.osm", new source is "mapzen.osm" but this requires different style asset package.
         * @param styleAssetPackage The style asset package (usually a zipped file or an asset)
         */
        CartoOnlineVectorTileLayer(const std::string& source, const std::shared_ptr<AssetPackage>& styleAssetPackage);
        virtual ~CartoOnlineVectorTileLayer();
        
    private:
        static std::shared_ptr<TileDataSource> CreateDataSource(const std::string& source);

        static std::shared_ptr<VectorTileDecoder> CreateTileDecoder(const std::shared_ptr<AssetPackage>& styleAssetPackage);
    };
    
}

#endif
