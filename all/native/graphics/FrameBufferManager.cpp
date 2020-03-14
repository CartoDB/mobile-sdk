#include "FrameBufferManager.h"
#include "graphics/FrameBuffer.h"
#include "utils/Log.h"

namespace carto {

    FrameBufferManager::FrameBufferManager() :
        _glThreadId(),
        _createQueue(),
        _deleteFBOIdQueue(),
        _deleteTexIdQueue(),
        _deleteRBIdQueue(),
        _mutex()
    {
    }
    
    FrameBufferManager::~FrameBufferManager() {
    }

    std::thread::id FrameBufferManager::getGLThreadId() const {
        std::lock_guard<std::mutex> lock(_mutex);

        return _glThreadId;
    }
    
    void FrameBufferManager::setGLThreadId(std::thread::id id) {
        std::lock_guard<std::mutex> lock(_mutex);

        _glThreadId = id;
    }
    
    std::shared_ptr<FrameBuffer> FrameBufferManager::createFrameBuffer(int width, int height, bool color, bool depth, bool stencil) {
        std::lock_guard<std::mutex> lock(_mutex);

        std::shared_ptr<FrameBuffer> frameBuffer(
            new FrameBuffer(shared_from_this(), width, height, color, depth, stencil), [this](FrameBuffer* frameBuffer) {
                deleteFrameBuffer(frameBuffer);
            }
        );

        _createQueue.push_back(frameBuffer);
        return frameBuffer;
    }

    void FrameBufferManager::processFrameBuffers() {
        std::vector<std::weak_ptr<FrameBuffer> > createQueue;
        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (std::this_thread::get_id() != _glThreadId) {
                Log::Warn("FrameBufferManager::processFrameBuffers: Method called from wrong thread!");
                return;
            }

            if (!_deleteFBOIdQueue.empty()) {
                glDeleteFramebuffers(static_cast<unsigned int>(_deleteFBOIdQueue.size()), _deleteFBOIdQueue.data());
                _deleteFBOIdQueue.clear();
            }
            if (!_deleteTexIdQueue.empty()) {
                glDeleteTextures(static_cast<unsigned int>(_deleteTexIdQueue.size()), _deleteTexIdQueue.data());
                _deleteTexIdQueue.clear();
            }
            if (!_deleteRBIdQueue.empty()) {
                glDeleteRenderbuffers(static_cast<unsigned int>(_deleteRBIdQueue.size()), _deleteRBIdQueue.data());
                _deleteRBIdQueue.clear();
            }

            for (const std::weak_ptr<FrameBuffer>& frameBufferWeak : _createQueue) {
                if (auto frameBuffer = frameBufferWeak.lock()) {
                    frameBuffer->create();
                }
            }
            std::swap(createQueue, _createQueue); // release the textures only after lock is released
        }

        GLContext::CheckGLError("FrameBufferManager::processFrameBuffers");
    }

    void FrameBufferManager::deleteFrameBuffer(FrameBuffer* frameBuffer) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (frameBuffer) {
            if (std::this_thread::get_id() == _glThreadId) {
                frameBuffer->destroy();
            } else {
                if (frameBuffer->_fboId != 0) {
                    _deleteFBOIdQueue.push_back(frameBuffer->_fboId);
                }
                if (frameBuffer->_colorTexId != 0) {
                    _deleteTexIdQueue.push_back(frameBuffer->_colorTexId);
                }
                _deleteRBIdQueue.insert(_deleteRBIdQueue.end(), frameBuffer->_depthStencilRBIds.begin(), frameBuffer->_depthStencilRBIds.end());
            }
            delete frameBuffer;
        }
    }

}
