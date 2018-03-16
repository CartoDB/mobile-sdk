/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_FRAMEBUFFERMANAGER_H_
#define _CARTO_FRAMEBUFFERMANAGER_H_

#include "graphics/FrameBuffer.h"

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace carto {

    class FrameBufferManager : public std::enable_shared_from_this<FrameBufferManager> {
    public:
        FrameBufferManager();
        virtual ~FrameBufferManager();

        std::thread::id getGLThreadId() const;
        void setGLThreadId(std::thread::id id);
    
        std::shared_ptr<FrameBuffer> createFrameBuffer(int width, int height, bool color, bool depth, bool stencil);

        void processFrameBuffers();
    
    private:
        void deleteFrameBuffer(FrameBuffer* frameBuffer);

        std::thread::id _glThreadId;
        std::vector<std::weak_ptr<FrameBuffer> > _createQueue;
        std::vector<GLuint> _deleteFBOIdQueue;
        std::vector<GLuint> _deleteTexIdQueue;
        std::vector<GLuint> _deleteRBIdQueue;
        mutable std::mutex _mutex;
    };
    
}

#endif
