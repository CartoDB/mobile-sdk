/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TILELOADLISTENER_H_
#define _CARTO_TILELOADLISTENER_H_

namespace carto {

    /**
     * Interface for monitoring tile loading events.
     */
    class TileLoadListener {
    public:
        virtual ~TileLoadListener() { }

        /**
         * Listener method that gets called when all visible raster tiles have finished loading.
         */
        virtual void onVisibleTilesLoaded() { }
    
        /**
         * Listener method that gets called when all preloading raster tiles have finished loading.
         * This method gets called after onVisibleTilesLoaded() and only if preloading is enabled.
         */
        virtual void onPreloadingTilesLoaded() { }
    };
        
}

#endif
