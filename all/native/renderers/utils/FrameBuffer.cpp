#include "FrameBuffer.h"
#include "renderers/utils/GLResourceManager.h"
#include "utils/Log.h"

namespace carto {

    FrameBuffer::~FrameBuffer() {
    }

    int FrameBuffer::getWidth() const {
        return _width;
    }
    
    int FrameBuffer::getHeight() const {
        return _height;
    }
    
    bool FrameBuffer::isColor() const {
        return _depth;
    }
    
    bool FrameBuffer::isDepth() const {
        return _depth;
    }
    
    bool FrameBuffer::isStencil() const {
        return _stencil;
    }

    GLuint FrameBuffer::getFBOId() const {
        return _fboId;
    }

    GLuint FrameBuffer::getColorTexId() const {
        return _colorTexId;
    }
        
    void FrameBuffer::discard(bool color, bool depth, bool stencil) {
        if (GLContext::DISCARD_FRAMEBUFFER) {
            std::vector<GLenum> attachments;
            if (color) {
                attachments.push_back(GL_COLOR_ATTACHMENT0);
            }
            if (depth) {
                attachments.push_back(GL_DEPTH_ATTACHMENT);
            }
            if (stencil) {
                attachments.push_back(GL_STENCIL_ATTACHMENT);
            }
            GLContext::DiscardFramebufferEXT(GL_FRAMEBUFFER, static_cast<int>(attachments.size()), attachments.data());
        }
    }
        
    FrameBuffer::FrameBuffer(const std::weak_ptr<GLResourceManager>& manager, int width, int height, bool color, bool depth, bool stencil) :
        GLResource(manager),
        _width(width),
        _height(height),
        _color(color),
        _depth(depth),
        _stencil(stencil),
        _fboId(0),
        _colorTexId(0),
        _depthStencilRBIds()
    {
    }

    void FrameBuffer::create() {
        if (_fboId == 0) {
            GLint oldFBOId = 0;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBOId);

            glGenFramebuffers(1, &_fboId);
            glBindFramebuffer(GL_FRAMEBUFFER, _fboId);

            GLint oldRBId = 0;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldRBId);

            if (_depth && _stencil && GLContext::PACKED_DEPTH_STENCIL) {
                GLuint depthStencilRBId = 0;
                glGenRenderbuffers(1, &depthStencilRBId);
                glBindRenderbuffer(GL_RENDERBUFFER, depthStencilRBId);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, _width, _height);
                glBindRenderbuffer(GL_RENDERBUFFER, oldRBId);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthStencilRBId);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilRBId);
                _depthStencilRBIds.push_back(depthStencilRBId);
            } else {
                if (_depth) {
                    GLuint depthRBId = 0;
                    glGenRenderbuffers(1, &depthRBId);
                    glBindRenderbuffer(GL_RENDERBUFFER, depthRBId);
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _width, _height);
                    glBindRenderbuffer(GL_RENDERBUFFER, oldRBId);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBId);
                    _depthStencilRBIds.push_back(depthRBId);
                }
                if (_stencil) {
                    GLuint stencilRBId = 0;
                    glGenRenderbuffers(1, &stencilRBId);
                    glBindRenderbuffer(GL_RENDERBUFFER, stencilRBId);
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, _width, _height);
                    glBindRenderbuffer(GL_RENDERBUFFER, oldRBId);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRBId);
                    _depthStencilRBIds.push_back(stencilRBId);
                }
            }

            if (_color) {
                GLint oldTexId = 0;
                glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexId);

                glGenTextures(1, &_colorTexId);
                glBindTexture(GL_TEXTURE_2D, _colorTexId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindTexture(GL_TEXTURE_2D, oldTexId);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexId, 0);
            }

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                Log::Error("FrameBuffer::create: Framebuffer not complete");
            }

            glBindFramebuffer(GL_FRAMEBUFFER, oldFBOId);

            GLContext::CheckGLError("FrameBuffer::create");
        }
    }

    void FrameBuffer::destroy() {
        if (_fboId != 0) {
            glDeleteFramebuffers(1, &_fboId);
            _fboId = 0;

            if (!_depthStencilRBIds.empty()) {
                glDeleteRenderbuffers(static_cast<int>(_depthStencilRBIds.size()), _depthStencilRBIds.data());
                _depthStencilRBIds.clear();
            }

            if (_colorTexId != 0) {
                glDeleteTextures(1, &_colorTexId);
                _colorTexId = 0;
            }

            GLContext::CheckGLError("FrameBuffer::destroy");
        }
    }
    
}
