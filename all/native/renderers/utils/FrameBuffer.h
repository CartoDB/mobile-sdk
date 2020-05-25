/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_FRAMEBUFFER_H_
#define _CARTO_FRAMEBUFFER_H_

#include "renderers/utils/GLResource.h"

#include <memory>
#include <vector>

namespace carto {
    
    class FrameBuffer : public GLResource {
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
        friend GLResourceManager;

        FrameBuffer(const std::weak_ptr<GLResourceManager>& manager, int width, int height, bool color, bool depth, bool stencil);

        virtual void create();
        virtual void destroy();

    private:
        int _width;
        int _height;
        bool _color;
        bool _depth;
        bool _stencil;
    
        GLuint _fboId;
        GLuint _colorTexId;
        std::vector<GLuint> _depthStencilRBIds;
    };
    
}

#endif
