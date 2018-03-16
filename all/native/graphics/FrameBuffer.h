/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_FRAMEBUFFER_H_
#define _CARTO_FRAMEBUFFER_H_

#include "graphics/utils/GLContext.h"

#include <memory>
#include <vector>

namespace carto {
    class FrameBufferManager;
    
    class FrameBuffer {
    public:
        virtual ~FrameBuffer();
        
        int getWidth() const;
        int getHeight() const;

        bool isColor() const;
        bool isDepth() const;
        bool isStencil() const;

        GLuint getFBOId() const;
        GLuint getColorTexId() const;

        void discard(bool color, bool depth, bool stencil);

    protected:
        friend class FrameBufferManager;

        FrameBuffer(const std::shared_ptr<FrameBufferManager>& frameBufferManager, int width, int height, bool color, bool depth, bool stencil);

        void create() const;
        void destroy() const;

    private:
        int _width;
        int _height;
        bool _color;
        bool _depth;
        bool _stencil;
    
        mutable GLuint _fboId;
        mutable GLuint _colorTexId;
        mutable std::vector<GLuint> _depthStencilRBIds;

        std::shared_ptr<FrameBufferManager> _frameBufferManager;
    };
    
}

#endif
