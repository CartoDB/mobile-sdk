/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GLESCONTEXT_H_
#define _CARTO_GLESCONTEXT_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#include <angle_windowsstore.h>

namespace carto {

    /**
     * Windows-specific wrapper for EGL context and display objects.
     */
    class GLESContext {
    public:
        GLESContext();
        virtual ~GLESContext();

        EGLSurface CreateSurface(void* panelPtr, int renderSurfaceWidth, int renderSurfaceHeight);
        void DestroySurface(const EGLSurface surface);
        void MakeCurrent(const EGLSurface surface);
        bool SwapBuffers(const EGLSurface surface);
        void Reset();

    private:
        void Initialize();
        void Cleanup();

        EGLDisplay mEglDisplay;
        EGLContext mEglContext;
        EGLConfig  mEglConfig;
    };

}

#endif
