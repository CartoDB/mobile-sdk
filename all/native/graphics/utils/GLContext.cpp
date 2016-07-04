#include "GLContext.h"
#include "utils/GLES2.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

#include <string>

namespace carto {

    void GLContext::CheckGLError(const std::string& place) {
        for (GLint error = glGetError(); error; error = glGetError()) {
            Log::Errorf("GLContext::CheckGLError: GLError (0x%x) at %s \n", error, place.c_str());
        }
    }
    
    bool GLContext::HasGLExtension(const std::string& extension) {
        std::lock_guard<std::mutex> lock(_Mutex);
    
        auto it = _ExtensionCache.find(extension);
        if (it != _ExtensionCache.end()) {
            return true;
        }
        return false;
    }
    
    void GLContext::LoadExtensions() {
        const char* extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
        if (!extensions) {
            return;
        }
    
        std::vector<std::string> tokens = GeneralUtils::Split(std::string(extensions), ' ');
        {
            std::lock_guard<std::mutex> lock(_Mutex);
            for (const std::string& extension : tokens) {
                _ExtensionCache.insert(extension);
            }
        }
        
        TEXTURE_FILTER_ANISOTROPIC = HasGLExtension("GL_EXT_texture_filter_anisotropic");
        TEXTURE_NPOT_REPEAT = HasGLExtension("GL_OES_texture_npot");
        TEXTURE_NPOT_MIPMAPS = HasGLExtension("GL_OES_texture_npot") || HasGLExtension("NV_texture_npot_2D_mipmap");
    }
        
    GLContext::GLContext() {
    }
    
    bool GLContext::TEXTURE_FILTER_ANISOTROPIC = false;
    bool GLContext::TEXTURE_NPOT_REPEAT = false;
    bool GLContext::TEXTURE_NPOT_MIPMAPS = false;
    
    std::size_t GLContext::MAX_VERTEXBUFFER_SIZE = 65535; // Should NOT exceed 64k!

    std::unordered_set<std::string> GLContext::_ExtensionCache;
        
    std::mutex GLContext::_Mutex;
    
}
