#include "GLExtensions.h"

#ifndef __APPLE__
#include <EGL/egl.h>
#endif

namespace carto { namespace vt {
    GLExtensions::GLExtensions() {
        std::string paddedExtensions;
        const char* extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        if (extensions) {
            paddedExtensions = " " + std::string(extensions) + " ";
        }

#ifdef GL_OES_vertex_array_object
#ifdef __ANDROID__
        _GL_OES_vertex_array_object_supported = false; // Android is a wild-wild west of GL implementations, at least one instance is known (Asus MemoPad) where extension support is reported but does not actually work
#else
        _GL_OES_vertex_array_object_supported = paddedExtensions.find(" GL_OES_vertex_array_object ") != std::string::npos;
#endif
#endif

#ifndef __APPLE__
        if (_GL_OES_vertex_array_object_supported) {
            _glBindVertexArrayOES = reinterpret_cast<PFNGLBINDVERTEXARRAYOESPROC>(eglGetProcAddress("glBindVertexArrayOES"));
            _glDeleteVertexArraysOES = reinterpret_cast<PFNGLDELETEVERTEXARRAYSOESPROC>(eglGetProcAddress("glDeleteVertexArraysOES"));
            _glGenVertexArraysOES = reinterpret_cast<PFNGLGENVERTEXARRAYSOESPROC>(eglGetProcAddress("glGenVertexArraysOES"));
            _glIsVertexArrayOES = reinterpret_cast<PFNGLISVERTEXARRAYOESPROC>(eglGetProcAddress("glIsVertexArrayOES"));
            _GL_OES_vertex_array_object_supported = _glBindVertexArrayOES && _glDeleteVertexArraysOES && _glGenVertexArraysOES && _glIsVertexArrayOES;
        }
#endif

#ifdef GL_EXT_discard_framebuffer
        _GL_EXT_discard_framebuffer_supported = paddedExtensions.find(" GL_EXT_discard_framebuffer ") != std::string::npos;
#endif

#ifndef __APPLE__
        if (_GL_EXT_discard_framebuffer_supported) {
            _glDiscardFramebufferEXT = reinterpret_cast<PFNGLDISCARDFRAMEBUFFEREXTPROC>(eglGetProcAddress("glDiscardFramebufferEXT"));
        }
#endif

#ifdef GL_EXT_texture_filter_anisotropic
        _GL_EXT_texture_filter_anisotropic_supported = paddedExtensions.find(" GL_EXT_texture_filter_anisotropic ") != std::string::npos;
#endif

#ifdef GL_OES_packed_depth_stencil
        _GL_OES_packed_depth_stencil_supported = paddedExtensions.find(" GL_OES_packed_depth_stencil ") != std::string::npos;
#endif
    }

    void GLExtensions::glBindVertexArrayOES(GLuint array) {
#ifdef __APPLE__
        ::glBindVertexArrayOES(array);
#else
        _glBindVertexArrayOES(array);
#endif
    }
    void GLExtensions::glDeleteVertexArraysOES(GLsizei n, const GLuint* arrays) {
#ifdef __APPLE__
        ::glDeleteVertexArraysOES(n, arrays);
#else
        _glDeleteVertexArraysOES(n, arrays);
#endif
    }

    void GLExtensions::glGenVertexArraysOES(GLsizei n, GLuint* arrays) {
#ifdef __APPLE__
        ::glGenVertexArraysOES(n, arrays);
#else
        _glGenVertexArraysOES(n, arrays);
#endif
    }

    GLboolean GLExtensions::glIsVertexArrayOES(GLuint array) {
#ifdef __APPLE__
        return ::glIsVertexArrayOES(array);
#else
        return _glIsVertexArrayOES(array);
#endif
    }

    void GLExtensions::glDiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum* attachments) {
#ifdef __APPLE__
        return ::glDiscardFramebufferEXT(target, numAttachments, attachments);
#else
        return _glDiscardFramebufferEXT(target, numAttachments, attachments);
#endif
    }
} }
