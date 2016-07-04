/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GLCONTEXT_H_
#define _CARTO_GLCONTEXT_H_

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
    
        static void CheckGLError(const std::string& place);
    
        static bool HasGLExtension(const std::string& extension);
    
        static void LoadExtensions();
        
    private:
        GLContext();

        static std::unordered_set<std::string> _ExtensionCache;
    
        static std::mutex _Mutex;
    };
    
}

#endif
