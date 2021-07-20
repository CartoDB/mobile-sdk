/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPEVENTLISTENER_H_
#define _CARTO_MAPEVENTLISTENER_H_

#include <memory>

namespace carto {
    class MapClickInfo;
    class MapInteractionInfo;
    
    /**
     * Listener for events like map clicks etc.
     */
    class MapEventListener {
    public:
        virtual ~MapEventListener() { }
    
        /**
         * Listener method that gets called at the end of the rendering process when the
         * map view needs no further refreshing.
         * Note that there can still be background processes (tile loading) that may change
         * the map view but these may take long time.
         * This method is called from GL renderer thread, not from main thread.
         */
        virtual void onMapIdle() { }

        /**
         * Listener method that gets called when the map is panned, rotated, tilted or zoomed.
         * The callback is used for both UI events and map changes resulting from API calls.
         * It is recommended to use onMapInteraction callback instead of onMapMoved, if possible.
         * Doing any calls to update MapView state from this method is potentially dangerous and may
         * result in deadlocks or crashes.
         * The thread this method is called from may vary.
         */
        virtual void onMapMoved() { }

        /**
         * Listener method that gets called when map is in 'stable' state - map animations have finished,
         * user has lifted fingers from the screen. This method is similar to onMapIdle, but is called less
         * frequently and takes account touch state.
         * The thread this method is called from may vary.
         */
        virtual void onMapStable() { }
    
        /**
         * Listener method that gets called when user has interacted with the map. The callback
         * includes info about interaction type (panning, zooming, etc).
         * @param mapInteractionInfo A container that provides information about the interaction.
         */
        virtual void onMapInteraction(const std::shared_ptr<MapInteractionInfo>& mapInteractionInfo) { }
        
        /**
         * Listener method that gets called when a click is performed on an empty area of the map.
         * This method will NOT be called from the main thread.
         * @param mapClickInfo A container that provides information about the click.
         */
        virtual void onMapClicked(const std::shared_ptr<MapClickInfo>& mapClickInfo) { }
    };
    
}

#endif
