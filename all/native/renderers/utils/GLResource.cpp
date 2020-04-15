#include "GLResource.h"
#include "renderers/utils/GLResourceManager.h"

#include <thread>

namespace carto {

    GLResource::~GLResource() {
    }

    bool GLResource::isValid() const {
        return std::this_thread::get_id() == _manager->getGLThreadId();
    }
      
    GLResource::GLResource(const std::shared_ptr<GLResourceManager>& manager) :
        _manager(manager)
    {
    }
    
}
