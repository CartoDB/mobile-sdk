#include "GLResource.h"
#include "renderers/utils/GLResourceManager.h"

#include <thread>

namespace carto {

    GLResource::~GLResource() {
    }

    bool GLResource::isValid() const {
        if (_manager.lock()) {
            return true;
        }
        return false;
    }
      
    GLResource::GLResource(const std::weak_ptr<GLResourceManager>& manager) :
        _manager(manager)
    {
    }

}
