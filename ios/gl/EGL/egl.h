#pragma once

#ifdef USE_METALANGLE
#include <MetalANGLE/EGL/egl.h>
#else
#include <OpenGLES/ES2/glext.h>
#include <string.h>

static inline void (*eglGetProcAddress(const char* procname))(void) {
    if (strcmp(procname, "glDiscardFramebufferEXT") == 0) {
        return reinterpret_cast<void(*)(void)>(&::glDiscardFramebufferEXT);
    }
    if (strcmp(procname, "glBindVertexArrayOES") == 0) {
        return reinterpret_cast<void(*)(void)>(&::glBindVertexArrayOES);
    }
    if (strcmp(procname, "glDeleteVertexArraysOES") == 0) {
        return reinterpret_cast<void(*)(void)>(&::glDeleteVertexArraysOES);
    }
    if (strcmp(procname, "glGenVertexArraysOES") == 0) {
        return reinterpret_cast<void(*)(void)>(&::glGenVertexArraysOES);
    }
    if (strcmp(procname, "glIsVertexArrayOES") == 0) {
        return reinterpret_cast<void(*)(void)>(&::glIsVertexArrayOES);
    }
    return NULL;
}

#endif
