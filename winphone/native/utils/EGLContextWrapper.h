/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_EGLCONTEXTWRAPPER_H_
#define _CARTO_EGLCONTEXTWRAPPER_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#include <angle_windowsstore.h>

namespace carto {

    class EGLContextWrapper {
    public:
        EGLContextWrapper();
        virtual ~EGLContextWrapper();

        EGLSurface createSurface(void* panelPtr, int renderSurfaceWidth, int renderSurfaceHeight);
        void destroySurface(const EGLSurface surface);
        void makeCurrent(const EGLSurface surface);
        bool swapBuffers(const EGLSurface surface);
        void reset();

    private:
        void initialize();
        void cleanup();

        EGLDisplay _eglDisplay;
        EGLContext _eglContext;
        EGLConfig  _eglConfig;
    };

}

#endif
