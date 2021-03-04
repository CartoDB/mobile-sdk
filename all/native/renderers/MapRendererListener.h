/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPRENDERERLISTENER_H_
#define _CARTO_MAPRENDERERLISTENER_H_

namespace carto {

    /**
     * Listener for specific map renderer events.
     */
    class MapRendererListener {
    public:
        virtual ~MapRendererListener() { }
        
        /**
         * Listener method that gets called when the rendering surface initialized and its size is updated.
         * The method can be used to check when it is safe to call methods that depend on view size,
         * like moveToFitBounds, screenToMap, mapToScreen.
         * This method is called from GL renderer thread, not from main thread.
         */
        virtual void onSurfaceChanged(int width, int height) { }
        
        /**
         * Listener method that gets called at the start of the rendering frame.
         * The method can be used to synchronize vector elements with renderer state, for example
         * to force marker to be always at the center of the screen (focus point).
         * This method is called from GL renderer thread, not from main thread.
         */
        virtual void onBeforeDrawFrame() { }
        
        /**
         * Listener method that gets called at the end of the rendering frame.
         * This method is called from GL renderer thread, not from main thread.
         */
        virtual void onAfterDrawFrame() { }
    };
    
}

#endif
