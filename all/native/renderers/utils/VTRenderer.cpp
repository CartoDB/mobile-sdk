#include "VTRenderer.h"
#include "renderers/utils/GLResourceManager.h"
#include "utils/Const.h"
#include "utils/Log.h"

namespace carto {

    VTRenderer::~VTRenderer() {
    }

    std::shared_ptr<vt::GLTileRenderer> VTRenderer::getTileRenderer() const {
        return _tileRenderer;
    }

    VTRenderer::VTRenderer(const std::weak_ptr<GLResourceManager>& manager, const std::shared_ptr<vt::TileTransformer>& tileTransformer) :
        GLResource(manager),
        _tileTransformer(tileTransformer),
        _tileRenderer()
    {
    }

    void VTRenderer::create() {
        if (!_tileRenderer) {
            Log::Debug("VTRenderer::create: Creating renderer");

            auto extensions = std::make_shared<vt::GLExtensions>();
            _tileRenderer = std::make_shared<vt::GLTileRenderer>(extensions, _tileTransformer, Const::WORLD_SIZE);
            _tileRenderer->initializeRenderer();

            GLContext::CheckGLError("VTRenderer::create");
        }
    }

    void VTRenderer::destroy() {
        if (_tileRenderer) {
            Log::Debug("VTRenderer::destroy: Releasing renderer");

            _tileRenderer->deinitializeRenderer();
            _tileRenderer.reset();

            GLContext::CheckGLError("VTRenderer::destroy");
        }
    }

}
