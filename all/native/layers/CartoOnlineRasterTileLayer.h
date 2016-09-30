/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOONLINERASTERTILELAYER_H_
#define _CARTO_CARTOONLINERASTERTILELAYER_H_

#include "layers/RasterTileLayer.h"

#include <string>
#include <memory>

namespace carto {
    /**
     * Specialized online raster tile layer that connects to Carto online tile server.
     * This layer is intended as a 'shortcut' to make using Carto online service and
     * raster tiles as simple as possible.
     */
    class CartoOnlineRasterTileLayer : public RasterTileLayer {
    public:
        /**
         * Constructs a CartoOnlineRasterrTileLayer object from a source name.
         * @param source The tile source name.
         */
        explicit CartoOnlineRasterTileLayer(const std::string& source);
        virtual ~CartoOnlineRasterTileLayer();
        
    private:
        static std::shared_ptr<TileDataSource> CreateDataSource(const std::string& source);
    };
    
}

#endif
