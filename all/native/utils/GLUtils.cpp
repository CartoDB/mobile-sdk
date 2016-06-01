#include "GLUtils.h"
#include "utils/GLES2.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

#include <string>

namespace carto {

    void GLUtils::checkGLError(const std::string& place) {
        for (GLint error = glGetError(); error; error = glGetError()) {
            Log::Errorf("GLError (0x%x) at %s \n", error, place.c_str());
        }
    }
    
    bool GLUtils::hasGLExtension(const std::string& extension) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        ExtensionCache::const_iterator it = _extensionCache.find(extension);
        if (it != _extensionCache.end()) {
            return true;
        }
        return false;
    }
    
    void GLUtils::loadExtensions() {
        const char* extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
        if (!extensions) {
            return;
        }
    
        std::vector<std::string> tokens = GeneralUtils::Split(std::string(extensions), ' ');
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for (const std::string& extension : tokens) {
                _extensionCache.insert(extension);
            }
        }
        
        _textureFilterAnisotropic = hasGLExtension("GL_EXT_texture_filter_anisotropic");
        _textureNPOTRepeat = hasGLExtension("GL_OES_texture_npot");
        _textureNPOTMipmaps = hasGLExtension("GL_OES_texture_npot") || hasGLExtension("NV_texture_npot_2D_mipmap");
    }
        
    bool GLUtils::isTextureFilterAnisotropic() {
        return _textureFilterAnisotropic;
    }
    
    bool GLUtils::isTextureNPOTRepeat() {
        return _textureNPOTRepeat;
    }
        
    bool GLUtils::isTextureNPOTMipmaps() {
        return _textureNPOTMipmaps;
    }

    GLUtils::GLUtils() {
    }
    
    const std::size_t GLUtils::MAX_VERTEXBUFFER_SIZE = 65535; // Should NOT exceed 64k!
    
    GLUtils::ExtensionCache GLUtils::_extensionCache;
        
    std::atomic<bool> GLUtils::_textureFilterAnisotropic(false);
    std::atomic<bool> GLUtils::_textureNPOTRepeat(false);
    std::atomic<bool> GLUtils::_textureNPOTMipmaps(false);
    
    std::mutex GLUtils::_mutex;
    
}
