/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_RASTERTILEEVENTLISTENER_H_
#define _CARTO_RASTERTILEEVENTLISTENER_H_

#include <memory>

namespace carto {
    class RasterTileClickInfo;
    
    /**
     * Listener for raster tile element events like clicks etc.
     */
    class RasterTileEventListener {
    public:
        virtual ~RasterTileEventListener() { }
    
        /**
         * Listener method that gets called when a click is performed on a raster tile.
         * This method will NOT be called from the main thread.
         * @param clickInfo A container that provides information about the click.
         * @return True if the click is handled and subsequent elements should not be handled. False if the next element should be called.
         */
        virtual bool onRasterTileClicked(const std::shared_ptr<RasterTileClickInfo>& clickInfo) { return true; }
    };
    
}

#endif
