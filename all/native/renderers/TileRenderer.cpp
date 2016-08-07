#include "TileRenderer.h"
#include "components/ThreadWorker.h"
#include "graphics/Frustum.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/ViewState.h"
#include "graphics/utils/GLContext.h"
#include "renderers/MapRenderer.h"
#include "renderers/drawdatas/TileDrawData.h"
#include "utils/Log.h"
#include "utils/Const.h"

#include <vt/TileLabelCuller.h>
#include <vt/GLTileRenderer.h>
#include <vt/GLExtensions.h>

#include <cglib/mat.h>

namespace {
    
    struct GLTileRendererDeleter : carto::ThreadWorker {
        GLTileRendererDeleter(std::unique_ptr<carto::vt::GLTileRenderer> renderer) : _renderer(std::move(renderer)) { }
        
        virtual void operator () () {
            _renderer->deinitializeRenderer();
        }
        
    private:
        std::unique_ptr<carto::vt::GLTileRenderer> _renderer;
    };

}
    
namespace carto {
    
    TileRenderer::TileRenderer(const std::weak_ptr<MapRenderer>& mapRenderer, bool useFBO, bool useDepth, bool useStencil) :
        _mapRenderer(mapRenderer),
        _glRenderer(),
        _glRendererMutex(std::make_shared<std::mutex>()),
        _useFBO(useFBO),
        _useDepth(useDepth),
        _useStencil(useStencil),
        _labelOrder(0),
        _buildingOrder(1),
        _horizontalLayerOffset(0),
        _tiles(),
        _mutex()
    {
    }
    
    TileRenderer::~TileRenderer() {
    }
    
    void TileRenderer::setLabelOrder(int order) {
        std::lock_guard<std::mutex> lock(_mutex);
        _labelOrder = order;
    }
    
    void TileRenderer::setBuildingOrder(int order) {
        std::lock_guard<std::mutex> lock(_mutex);
        _buildingOrder = order;
    }
    
    void TileRenderer::offsetLayerHorizontally(double offset) {
        std::lock_guard<std::mutex> lock(_mutex);
        _horizontalLayerOffset += offset;
    }
    
    void TileRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        Log::Debug("TileRenderer: Surface created");

        std::weak_ptr<MapRenderer> mapRendererWeak(_mapRenderer);
        auto glRendererDeleter = [mapRendererWeak](vt::GLTileRenderer* rendererPtr) {
            std::unique_ptr<vt::GLTileRenderer> renderer(rendererPtr);
            Log::Debug("TileRenderer: Releasing renderer resources");
            if (auto mapRenderer = mapRendererWeak.lock()) {
                mapRenderer->addRenderThreadCallback(std::make_shared<GLTileRendererDeleter>(std::move(renderer)));
            }
        };
        _glRenderer = std::shared_ptr<vt::GLTileRenderer>(
            new vt::GLTileRenderer(_glRendererMutex, std::make_shared<carto::vt::GLExtensions>(), carto::Const::WORLD_SIZE, _useFBO, _useDepth, _useStencil), glRendererDeleter
        );
        _glRenderer->setSubTileBlending(true);
        _glRenderer->initializeRenderer();
        _tiles.clear();
        GLContext::CheckGLError("TileRenderer::onSurfaceCreated()");
    }
    
    bool TileRenderer::onDrawFrame(float deltaSeconds, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (!_glRenderer) {
            return false;
        }

        cglib::mat4x4<double> modelViewMat = viewState.getModelviewMat();
        modelViewMat = modelViewMat * cglib::translate4_matrix(cglib::vec3<double>(_horizontalLayerOffset, 0, 0));
        _glRenderer->setViewState(viewState.getProjectionMat(), modelViewMat, viewState.getZoom(), viewState.getAspectRatio(), viewState.getNormalizedResolution());
        
        _glRenderer->startFrame(deltaSeconds * 3);

        bool refresh = _glRenderer->render2D();
        if (_labelOrder == 0) {
            refresh = _glRenderer->renderLabels(true, false) || refresh;
        }
        if (_buildingOrder == 0) {
            refresh = _glRenderer->render3D() || refresh;
        }
        if (_labelOrder == 0) {
            refresh = _glRenderer->renderLabels(false, true) || refresh;
        }
    
        // Reset GL state to the expected state
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        GLContext::CheckGLError("TileRenderer::onDrawFrame()");
        return refresh;
    }
    
    bool TileRenderer::onDrawFrame3D(float deltaSeconds, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (!_glRenderer) {
            return false;
        }

        bool refresh = false;	
        if (_labelOrder == 1) {
            refresh = _glRenderer->renderLabels(true, false) || refresh;
        }
        if (_buildingOrder == 1) {
            refresh = _glRenderer->render3D() || refresh;
        }
        if (_labelOrder == 1) {
            refresh = _glRenderer->renderLabels(false, true) || refresh;
        }

        _glRenderer->endFrame();

        // Reset GL state to the expected state
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        GLContext::CheckGLError("TileRenderer::onDrawFrame3D()");
        return refresh;
    }
    
    void TileRenderer::onSurfaceDestroyed() {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_glRenderer) {
            return;
        }

        Log::Debug("TileRenderer: Surface destroyed");

        _glRenderer->resetRenderer();
        _glRenderer.reset();
    }
    
    void TileRenderer::setBackgroundColor(const Color& color) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_glRenderer) {
            return;
        }

        if (_useFBO) {
            _glRenderer->setFBOClearColor(vt::Color(color.getARGB()));
        }
        else {
            _glRenderer->setBackgroundColor(vt::Color(color.getARGB()));
        }
    }
    
    void TileRenderer::setBackgroundPattern(const std::shared_ptr<const vt::BitmapPattern>& pattern) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_glRenderer) {
            return;
        }

        _glRenderer->setBackgroundPattern(pattern);
    }
    
    bool TileRenderer::cullLabels(const ViewState& viewState) {
        cglib::mat4x4<double> modelViewMat = viewState.getModelviewMat();
        std::vector<std::shared_ptr<vt::TileLabel> > visibleLabels;
        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (!_glRenderer) {
                return false;
            }

            modelViewMat = modelViewMat * cglib::translate4_matrix(cglib::vec3<double>(_horizontalLayerOffset, 0, 0));
            visibleLabels = _glRenderer->getVisibleLabels();
        }

        vt::TileLabelCuller culler(_glRendererMutex, Const::WORLD_SIZE);
        culler.setViewState(viewState.getProjectionMat(), modelViewMat, viewState.getZoom(), viewState.getAspectRatio(), viewState.getNormalizedResolution());
        culler.process(visibleLabels);
        return true;
    }
    
    bool TileRenderer::refreshTiles(const std::vector<std::shared_ptr<TileDrawData> >& drawDatas) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_glRenderer) {
            return false;
        }

        std::map<vt::TileId, std::shared_ptr<const vt::Tile> > tiles;
        for (const std::shared_ptr<TileDrawData>& drawData : drawDatas) {
            tiles[drawData->getVTTileId()] = drawData->getVTTile();
        }

        bool changed = tiles != _tiles;
        if (changed) {
            _glRenderer->setVisibleTiles(tiles, _horizontalLayerOffset == 0);
            _tiles = tiles;
        }
        _horizontalLayerOffset = 0;
        return changed;
    }
        
}
