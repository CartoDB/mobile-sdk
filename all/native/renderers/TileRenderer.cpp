#include "TileRenderer.h"
#include "components/Options.h"
#include "components/ThreadWorker.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/ViewState.h"
#include "graphics/utils/GLContext.h"
#include "projections/PlanarProjectionSurface.h"
#include "renderers/MapRenderer.h"
#include "renderers/drawdatas/TileDrawData.h"
#include "utils/Log.h"
#include "utils/Const.h"

#include <vt/Label.h>
#include <vt/TileTransformer.h>
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
    
    TileRenderer::TileRenderer(const std::weak_ptr<MapRenderer>& mapRenderer, const std::shared_ptr<vt::TileTransformer>& tileTransformer) :
        _mapRenderer(mapRenderer),
        _tileTransformer(tileTransformer),
        _glRenderer(),
        _interactionMode(false),
        _subTileBlending(true),
        _labelOrder(0),
        _buildingOrder(1),
        _horizontalLayerOffset(0),
        _lightDir(0, 0, 0),
        _tiles(),
        _options(),
        _mutex()
    {
    }
    
    TileRenderer::~TileRenderer() {
    }
    
    void TileRenderer::setOptions(const std::weak_ptr<Options>& options) {
        std::lock_guard<std::mutex> lock(_mutex);
        _options = options;
    }
    
    void TileRenderer::setInteractionMode(bool enabled) {
        std::lock_guard<std::mutex> lock(_mutex);
        _interactionMode = enabled;
    }
    
    void TileRenderer::setSubTileBlending(bool enabled) {
        std::lock_guard<std::mutex> lock(_mutex);
        _subTileBlending = enabled;
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

        boost::optional<vt::GLTileRenderer::LightingShader> lightingShader2D;
        if (auto mapRenderer = _mapRenderer.lock()) {
            if (!std::dynamic_pointer_cast<PlanarProjectionSurface>(mapRenderer->getProjectionSurface())) {
                lightingShader2D = vt::GLTileRenderer::LightingShader(true, LIGHTING_SHADER_2D, [this](GLuint shaderProgram, const vt::ViewState& viewState) {
                    glUniform3fv(glGetUniformLocation(shaderProgram, "u_lightDir"), 1, _lightDir.data());
                });
            }
        }
        boost::optional<vt::GLTileRenderer::LightingShader> lightingShader3D = vt::GLTileRenderer::LightingShader(true, LIGHTING_SHADER_3D, [this](GLuint shaderProgram, const vt::ViewState& viewState) {
            if (auto options = _options.lock()) {
                const Color& ambientLightColor = options->getAmbientLightColor();
                glUniform4f(glGetUniformLocation(shaderProgram, "u_ambientColor"), ambientLightColor.getR() / 255.0f, ambientLightColor.getG() / 255.0f, ambientLightColor.getB() / 255.0f, ambientLightColor.getA() / 255.0f);
                const Color& mainLightColor = options->getMainLightColor();
                glUniform4f(glGetUniformLocation(shaderProgram, "u_lightColor"), mainLightColor.getR() / 255.0f, mainLightColor.getG() / 255.0f, mainLightColor.getB() / 255.0f, mainLightColor.getA() / 255.0f);
                const MapVec& mainLightDir = options->getMainLightDirection();
                glUniform3f(glGetUniformLocation(shaderProgram, "u_lightDir"), static_cast<float>(mainLightDir.getX()), static_cast<float>(mainLightDir.getY()), static_cast<float>(mainLightDir.getZ()));
            }
        });

        std::weak_ptr<MapRenderer> mapRendererWeak(_mapRenderer);
        auto glRendererDeleter = [mapRendererWeak](vt::GLTileRenderer* rendererPtr) {
            std::unique_ptr<vt::GLTileRenderer> renderer(rendererPtr);
            Log::Debug("TileRenderer: Releasing renderer resources");
            if (auto mapRenderer = mapRendererWeak.lock()) {
                mapRenderer->addRenderThreadCallback(std::make_shared<GLTileRendererDeleter>(std::move(renderer)));
            }
        };
        _glRenderer = std::shared_ptr<vt::GLTileRenderer>(
            new vt::GLTileRenderer(std::make_shared<vt::GLExtensions>(), _tileTransformer, lightingShader2D, lightingShader3D, Const::WORLD_SIZE), glRendererDeleter
        );
        _glRenderer->initializeRenderer();
        _tiles.clear();
        GLContext::CheckGLError("TileRenderer::onSurfaceCreated");
    }
    
    bool TileRenderer::onDrawFrame(float deltaSeconds, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (!_glRenderer) {
            return false;
        }

        cglib::mat4x4<double> modelViewMat = viewState.getModelviewMat() * cglib::translate4_matrix(cglib::vec3<double>(_horizontalLayerOffset, 0, 0));
        _glRenderer->setViewState(vt::ViewState(viewState.getProjectionMat(), modelViewMat, viewState.getZoom(), viewState.getAspectRatio(), viewState.getNormalizedResolution()));
        _glRenderer->setInteractionMode(_interactionMode);
        _glRenderer->setSubTileBlending(_subTileBlending);

        _lightDir = viewState.getFocusPosNormal();

        _glRenderer->startFrame(deltaSeconds * 3);

        bool refresh = _glRenderer->renderGeometry2D();
        if (_labelOrder == 0) {
            refresh = _glRenderer->renderLabels(true, false) || refresh;
        }
        if (_buildingOrder == 0) {
            refresh = _glRenderer->renderGeometry3D() || refresh;
        }
        if (_labelOrder == 0) {
            refresh = _glRenderer->renderLabels(false, true) || refresh;
        }
    
        // Reset GL state to the expected state
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        GLContext::CheckGLError("TileRenderer::onDrawFrame");
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
            refresh = _glRenderer->renderGeometry3D() || refresh;
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

        GLContext::CheckGLError("TileRenderer::onDrawFrame3D");
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
    
    bool TileRenderer::cullLabels(const ViewState& viewState) {
        std::shared_ptr<vt::GLTileRenderer> glRenderer;
        cglib::mat4x4<double> modelViewMat;
        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (!_glRenderer) {
                return false;
            }

            glRenderer = _glRenderer;
            modelViewMat = viewState.getModelviewMat() * cglib::translate4_matrix(cglib::vec3<double>(_horizontalLayerOffset, 0, 0));
        }

        glRenderer->cullLabels(vt::ViewState(viewState.getProjectionMat(), modelViewMat, viewState.getZoom(), viewState.getAspectRatio(), viewState.getNormalizedResolution()));
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

    void TileRenderer::calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<std::tuple<vt::TileId, double, long long> >& results) const {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_glRenderer) {
            return;
        }

        float radius = CLICK_RADIUS; // NOTE: the value will be automatically multiplied with DPI factor

        _glRenderer->findGeometryIntersections(ray, results, radius, true, false);
        if (_labelOrder == 0) {
            _glRenderer->findLabelIntersections(ray, results, radius, true, false);
        }
        if (_buildingOrder == 0) {
            _glRenderer->findGeometryIntersections(ray, results, radius, false, true);
        }
        if (_labelOrder == 0) {
            _glRenderer->findLabelIntersections(ray, results, radius, false, true);
        }
    }
        
    void TileRenderer::calculateRayIntersectedBitmaps(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<std::tuple<vt::TileId, double, vt::TileBitmap, cglib::vec2<float> > >& results) const {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_glRenderer) {
            return;
        }

        _glRenderer->findBitmapIntersections(ray, results);
    }

    void TileRenderer::calculateRayIntersectedElements3D(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<std::tuple<vt::TileId, double, long long> >& results) const {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_glRenderer) {
            return;
        }

        float radius = viewState.getUnitToDPCoef() * CLICK_RADIUS;

        if (_labelOrder == 1) {
            _glRenderer->findLabelIntersections(ray, results, radius, true, false);
        }
        if (_buildingOrder == 1) {
            _glRenderer->findGeometryIntersections(ray, results, radius, false, true);
        }
        if (_labelOrder == 1) {
            _glRenderer->findLabelIntersections(ray, results, radius, false, true);
        }
    }

    const std::string TileRenderer::LIGHTING_SHADER_2D = R"GLSL(
        uniform vec3 u_lightDir;
        vec4 applyLighting(vec4 color, vec3 normal) {
            float lighting = max(0.0, dot(normal, u_lightDir)) * 0.5 + 0.5;
            return vec4(color.rgb * lighting, color.a);
        }
    )GLSL";

    const std::string TileRenderer::LIGHTING_SHADER_3D = R"GLSL(
        uniform vec4 u_ambientColor;
        uniform vec4 u_lightColor;
        uniform vec3 u_lightDir;
        vec4 applyLighting(vec4 color, vec3 normal) {
            vec3 lighting = max(0.0, dot(normal, u_lightDir)) * u_lightColor.rgb + u_ambientColor.rgb;
            return vec4(color.rgb * lighting, color.a);
        }
    )GLSL";

}
