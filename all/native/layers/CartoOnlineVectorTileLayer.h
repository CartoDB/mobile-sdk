/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOONLINEVECTORTILELAYER_H_
#define _CARTO_CARTOONLINEVECTORTILELAYER_H_

#include "layers/VectorTileLayer.h"

#include <string>

namespace carto {
    /**
     * Specialized online vector tile layer that connects to Carto online tile server.
     * This layer is intended as a 'shortcut' to make using Carto online service and
     * vector tiles as simple as possible.
     */
    class CartoOnlineVectorTileLayer : public VectorTileLayer {
    public:
        /**
         * Constructs a CartoOnlineVectorTileLayer object from a style asset name.
         * Uses "nutiteq.osm" as a source.
         * Style asset must be included in the project, style asset defines visual style of the map.
         * @param styleAssetName The name of the style asset that defines visual style of the map.
         */
        CartoOnlineVectorTileLayer(const std::string& styleAssetName);
        /**
         * Constructs a CartoOnlineVectorTileLayer object from a source name and style asset name.
         * Style asset must be included in the project, style asset defines visual style of the map.
         * @param source The tile source name. Main and default source is currently "nutiteq.osm".
         * @param styleAssetName The name of the style asset that defines visual style of the map.
         */
        CartoOnlineVectorTileLayer(const std::string& source, const std::string& styleAssetName);
        virtual ~CartoOnlineVectorTileLayer();
        
    private:
        static std::shared_ptr<TileDataSource> CreateDataSource(const std::string& source);
        static std::shared_ptr<VectorTileDecoder> CreateTileDecoder(const std::string& syleAssetName);
        
        static const std::string DEFAULT_SOURCE;
    };
    
}

#endif
