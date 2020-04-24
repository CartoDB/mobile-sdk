#include "GLResourceManager.h"
#include "utils/Log.h"

namespace carto {

    GLResourceManager::GLResourceManager() :
        _glThreadId(),
        _createQueue(),
        _deleteQueue(),
        _mutex()
    {
    }
    
    GLResourceManager::~GLResourceManager() {
        if (std::this_thread::get_id() == getGLThreadId()) {
            for (const std::unique_ptr<GLResource>& resource : _deleteQueue) {
                resource->destroy();
            }
            _deleteQueue.clear();
        }
        if (!_deleteQueue.empty()) {
            Log::Debugf("GLResourceManager::~GLResourceManager: Delete queue size: %d", static_cast<int>(_deleteQueue.size()));
        }
    }

    std::thread::id GLResourceManager::getGLThreadId() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _glThreadId;
    }
    
    void GLResourceManager::setGLThreadId(std::thread::id id) {
        std::lock_guard<std::mutex> lock(_mutex);
        _glThreadId = id;
    }
    
    void GLResourceManager::processResources() {
        if (std::this_thread::get_id() != getGLThreadId()) {
            Log::Warn("GLResourceManager::processResources: Method called from wrong thread!");
            return;
        }

        std::vector<std::weak_ptr<GLResource> > createQueue;
        std::vector<std::unique_ptr<GLResource> > deleteQueue;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::swap(deleteQueue, _deleteQueue);
            std::swap(createQueue, _createQueue);
        }
        for (const std::unique_ptr<GLResource>& resource : deleteQueue) {
            resource->destroy();
        }
        for (const std::weak_ptr<GLResource>& resourceWeak : createQueue) {
            if (auto resource = resourceWeak.lock()) {
                resource->create();
            }
        }
    }

    std::shared_ptr<GLResource> GLResourceManager::registerResource(GLResource* resourcePtr) {
        std::shared_ptr<GLResource> resource;
        try {
            std::shared_ptr<GLResourceManager> manager(shared_from_this());
            resource = std::shared_ptr<GLResource>(resourcePtr, [manager](GLResource* resourcePtr) {
                manager->deleteResource(std::unique_ptr<GLResource>(resourcePtr));
            });
        }
        catch (const std::exception&) {
            delete resourcePtr;
            throw;
        }

        if (std::this_thread::get_id() == getGLThreadId()) {
            resource->create();
        } else {
            std::lock_guard<std::mutex> lock(_mutex);
            _createQueue.push_back(resource);
        }
        return resource;
    }

    void GLResourceManager::deleteResource(std::unique_ptr<GLResource> resource) {
        if (resource) {
            if (std::this_thread::get_id() == getGLThreadId()) {
                resource->destroy();
            } else {
                std::lock_guard<std::mutex> lock(_mutex);
                _deleteQueue.push_back(std::move(resource));
            }
        }
    }

}
