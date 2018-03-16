#include "GLContext.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

#ifndef __APPLE__
#include <EGL/egl.h>
#endif

namespace carto {

    bool GLContext::HasGLExtension(const char* extension) {
        std::lock_guard<std::recursive_mutex> lock(_Mutex);
    
        auto it = _ExtensionCache.find(extension);
        if (it != _ExtensionCache.end()) {
            return true;
        }
        return false;
    }
    
    void GLContext::LoadExtensions() {
        std::lock_guard<std::recursive_mutex> lock(_Mutex);

        const char* extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        if (!extensions) {
            return;
        }
    
        std::vector<std::string> tokens = GeneralUtils::Split(std::string(extensions), ' ');
        for (const std::string& extension : tokens) {
            _ExtensionCache.insert(extension);
        }
        
        TEXTURE_FILTER_ANISOTROPIC = HasGLExtension("GL_EXT_texture_filter_anisotropic");
        TEXTURE_NPOT_REPEAT = HasGLExtension("GL_OES_texture_npot");
        TEXTURE_NPOT_MIPMAPS = HasGLExtension("GL_OES_texture_npot") || HasGLExtension("NV_texture_npot_2D_mipmap");

        DISCARD_FRAMEBUFFER = HasGLExtension("GL_EXT_discard_framebuffer");

        PACKED_DEPTH_STENCIL = HasGLExtension("GL_OES_packed_depth_stencil");

#if !defined(__APPLE__) && defined(GL_EXT_discard_framebuffer)
        if (DISCARD_FRAMEBUFFER) {
            _DiscardFramebufferEXT = reinterpret_cast<PFNGLDISCARDFRAMEBUFFEREXTPROC>(eglGetProcAddress("glDiscardFramebufferEXT"));
        }
#endif
    }
        
    void GLContext::CheckGLError(const char* place) {
        for (GLint error = glGetError(); error; error = glGetError()) {
            Log::Errorf("GLContext::CheckGLError: GLError (0x%x) at %s \n", error, place);
        }
    }

    void GLContext::DiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum* attachments) {
        std::lock_guard<std::recursive_mutex> lock(_Mutex);

#ifdef __APPLE__
        ::glDiscardFramebufferEXT(target, numAttachments, attachments);
#else
        if (_DiscardFramebufferEXT) {
            _DiscardFramebufferEXT(target, numAttachments, attachments);
        }
#endif
    }
    
    GLContext::GLContext() {
    }
    
    bool GLContext::TEXTURE_FILTER_ANISOTROPIC = false;
    bool GLContext::TEXTURE_NPOT_REPEAT = false;
    bool GLContext::TEXTURE_NPOT_MIPMAPS = false;

    bool GLContext::DISCARD_FRAMEBUFFER = false;

    bool GLContext::PACKED_DEPTH_STENCIL = false;
    
    std::size_t GLContext::MAX_VERTEXBUFFER_SIZE = 65535; // Should NOT exceed 64k!

#if !defined(__APPLE__) && defined(GL_EXT_discard_framebuffer)
    PFNGLDISCARDFRAMEBUFFEREXTPROC GLContext::_DiscardFramebufferEXT = nullptr;
#endif

    std::unordered_set<std::string> GLContext::_ExtensionCache;
        
    std::recursive_mutex GLContext::_Mutex;
    
}
