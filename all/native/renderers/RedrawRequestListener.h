/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_REDRAWREQUESTLISTENER_H_
#define _CARTO_REDRAWREQUESTLISTENER_H_

namespace carto {

    /**
     * An internal listener class for notifying about screen redraw requests.
     */
    class RedrawRequestListener {
    public:
        virtual ~RedrawRequestListener() { }
    
        /**
         * Called when the screen needs to be redrawn.
         */
        virtual void onRedrawRequested() const { }
    };
    
}

#endif
