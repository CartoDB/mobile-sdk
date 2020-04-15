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
        std::vector<std::weak_ptr<GLResource> > createQueue;
        std::vector<std::unique_ptr<GLResource> > deleteQueue;
        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (std::this_thread::get_id() != _glThreadId) {
                Log::Warn("GLResourceManager::processResources: Method called from wrong thread!");
                return;
            }

            for (const std::unique_ptr<GLResource>& resource : _deleteQueue) {
                resource->destroy();
            }
            std::swap(deleteQueue, _deleteQueue);

            for (const std::weak_ptr<GLResource>& resourceWeak : _createQueue) {
                if (auto resource = resourceWeak.lock()) {
                    resource->create();
                }
            }
            std::swap(createQueue, _createQueue); // release the resources only after lock is released
        }
    }

    std::shared_ptr<GLResource> GLResourceManager::registerResource(GLResource* resourcePtr) {
        std::shared_ptr<GLResource> resource;
        try {
            std::weak_ptr<GLResourceManager> managerWeak(shared_from_this());

            resource = std::shared_ptr<GLResource>(resourcePtr, [managerWeak](GLResource* resourcePtr) {
                std::unique_ptr<GLResource> resource(resourcePtr);
                if (auto manager = managerWeak.lock()) {
                    manager->deleteResource(std::move(resource));
                }
            });
        }
        catch (const std::exception&) {
            delete resourcePtr;
            throw;
        }

        std::lock_guard<std::mutex> lock(_mutex);

        if (std::this_thread::get_id() == _glThreadId) {
            resource->create();
        } else {
            _createQueue.push_back(resource);
        }
        return resource;
    }

    void GLResourceManager::deleteResource(std::unique_ptr<GLResource> resource) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (resource) {
            if (std::this_thread::get_id() == _glThreadId) {
                resource->destroy();
            } else {
                _deleteQueue.push_back(std::move(resource));
            }
        }
    }

}
