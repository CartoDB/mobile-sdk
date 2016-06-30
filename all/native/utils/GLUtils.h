/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GLUTILS_H_
#define _CARTO_GLUTILS_H_

#include <mutex>
#include <string>
#include <unordered_set>
#include <atomic>

namespace carto {

    class GLUtils {
    public:
        static const std::size_t MAX_VERTEXBUFFER_SIZE;
    
        static void checkGLError(const std::string& place);
    
        static bool hasGLExtension(const std::string& extension);
    
        static void loadExtensions();
        
        static bool isTextureFilterAnisotropic();
        static bool isTextureNPOTRepeat();
        static bool isTextureNPOTMipmaps();
    
    private:
        GLUtils();

        typedef std::unordered_set<std::string> ExtensionCache;
    
        static ExtensionCache _extensionCache;
        
        static std::atomic<bool> _textureFilterAnisotropic;
        static std::atomic<bool> _textureNPOTRepeat;
        static std::atomic<bool> _textureNPOTMipmaps;
        
        static std::mutex _mutex;
    };
    
}

#endif
