/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GLCONTEXT_H_
#define _CARTO_GLCONTEXT_H_

#ifdef __APPLE__
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#include <mutex>
#include <string>
#include <unordered_set>
#include <atomic>

namespace carto {

    class GLContext {
    public:
        static bool TEXTURE_FILTER_ANISOTROPIC;
        static bool TEXTURE_NPOT_REPEAT;
        static bool TEXTURE_NPOT_MIPMAPS;

        static std::size_t MAX_VERTEXBUFFER_SIZE;
    
        static bool HasGLExtension(const char* extension);
    
        static void LoadExtensions();
        
        static void CheckGLError(const char* place);
    
    private:
        GLContext();

        static std::unordered_set<std::string> _ExtensionCache;
    
        static std::mutex _Mutex;
    };
    
}

#endif
