/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_RENDERERCAPTURELISTENER_H_
#define _CARTO_RENDERERCAPTURELISTENER_H_

#include <memory>

namespace carto {
    class Bitmap;

    /**
     * Listener for renderer capture events.
     */
    class RendererCaptureListener {
    public:
        virtual ~RendererCaptureListener() { }
        
        /**
         * Listener method that is called when the view has been rendered.
         * @param bitmap The rendered view as a bitmap.
         */
        virtual void onMapRendered(const std::shared_ptr<Bitmap>& bitmap) { }
    };
    
}

#endif
