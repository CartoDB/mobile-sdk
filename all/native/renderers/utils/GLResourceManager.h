/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GLRESOURCEMANAGER_H_
#define _CARTO_GLRESOURCEMANAGER_H_

#include "renderers/utils/GLResource.h"

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace carto {

    class GLResourceManager : public std::enable_shared_from_this<GLResourceManager> {
    public:
        GLResourceManager();
        virtual ~GLResourceManager();

        std::thread::id getGLThreadId() const;
        void setGLThreadId(std::thread::id id);

        template <typename T, typename... Args>
        std::shared_ptr<T> create(Args&&... args) {
            return std::static_pointer_cast<T>(registerResource(new T(shared_from_this(), std::forward<Args>(args)...)));
        }
    
        void processResources();
    
    protected:
        std::shared_ptr<GLResource> registerResource(GLResource* resourcePtr);
        void deleteResource(std::unique_ptr<GLResource> resource);

    private:
        std::thread::id _glThreadId;
        std::vector<std::weak_ptr<GLResource> > _createQueue;
        std::vector<std::unique_ptr<GLResource> > _deleteQueue;
        mutable std::mutex _mutex;
    };
    
}

#endif
