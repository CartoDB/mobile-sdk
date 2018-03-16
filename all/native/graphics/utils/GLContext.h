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

        static bool DISCARD_FRAMEBUFFER;

        static bool PACKED_DEPTH_STENCIL;

        static std::size_t MAX_VERTEXBUFFER_SIZE;
    
        static bool HasGLExtension(const char* extension);
    
        static void LoadExtensions();
        
        static void CheckGLError(const char* place);

        static void DiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum* attachments);
    
    private:
        GLContext();

#if !defined(__APPLE__) && defined(GL_EXT_discard_framebuffer)
        static PFNGLDISCARDFRAMEBUFFEREXTPROC _DiscardFramebufferEXT;
#endif

        static std::unordered_set<std::string> _ExtensionCache;
    
        static std::recursive_mutex _Mutex;
    };
    
}

#endif
