#include "NMLResources.h"
#include "renderers/utils/GLResourceManager.h"
#include "utils/Log.h"

#include <nml/GLModel.h>
#include <nml/GLTexture.h>

namespace carto {

    NMLResources::~NMLResources() {
    }

    std::shared_ptr<nml::GLResourceManager> NMLResources::getResourceManager() const {
        return _resourceManager;
    }

    NMLResources::NMLResources(const std::weak_ptr<GLResourceManager>& manager) :
        GLResource(manager),
        _resourceManager()
    {
    }

    void NMLResources::create() {
        if (!_resourceManager) {
            Log::Debug("NMLResources::create: Creating renderer");

            nml::GLTexture::registerGLExtensions();
            _resourceManager = std::make_shared<nml::GLResourceManager>();

            GLContext::CheckGLError("NMLResources::create");
        }
    }

    void NMLResources::destroy() {
        if (_resourceManager) {
            Log::Debug("NMLResources::destroy: Releasing renderer");

            _resourceManager->deleteAll();
            _resourceManager.reset();

            GLContext::CheckGLError("NMLResources::destroy");
        }
    }

}
