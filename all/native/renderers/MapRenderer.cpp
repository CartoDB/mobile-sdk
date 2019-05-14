#include "MapRenderer.h"
#include "components/Exceptions.h"
#include "components/Layers.h"
#include "components/ThreadWorker.h"
#include "core/MapPos.h"
#include "core/ScreenPos.h"
#include "core/ScreenBounds.h"
#include "drawdatas/BillboardDrawData.h"
#include "graphics/Bitmap.h"
#include "graphics/FrameBuffer.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/FrameBufferManager.h"
#include "graphics/ShaderManager.h"
#include "graphics/TextureManager.h"
#include "graphics/utils/GLContext.h"
#include "layers/Layer.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "renderers/BillboardRenderer.h"
#include "renderers/MapRendererListener.h"
#include "renderers/RendererCaptureListener.h"
#include "renderers/RedrawRequestListener.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/cameraevents/CameraPanEvent.h"
#include "renderers/cameraevents/CameraRotationEvent.h"
#include "renderers/cameraevents/CameraTiltEvent.h"
#include "renderers/cameraevents/CameraZoomEvent.h"
#include "renderers/workers/BillboardPlacementWorker.h"
#include "renderers/workers/CullWorker.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/ThreadUtils.h"

#include <algorithm>

namespace carto {

    MapRenderer::MapRenderer(const std::shared_ptr<Layers>& layers, const std::shared_ptr<Options>& options) :
        _lastFrameTime(),
        _viewState(),
        _frameBufferManager(),
        _shaderManager(),
        _textureManager(),
        _styleCache(),
        _cullWorker(std::make_shared<CullWorker>()),
        _cullThread(),
        _optionsListener(),
        _currentBoundFBOs(),
        _screenFrameBuffer(),
        _screenBlendShader(),
        _backgroundRenderer(*options, *layers),
        _watermarkRenderer(*options),
        _billboardSorter(),
        _billboardDrawDataBuffer(),
        _billboardPlacementWorker(std::make_shared<BillboardPlacementWorker>()),
        _billboardPlacementThread(),
        _animationHandler(*this),
        _kineticEventHandler(*this, *options),
        _layers(layers),
        _options(options),
        _surfaceCreated(false),
        _surfaceChanged(false),
        _billboardsChanged(false),
        _renderProjectionChanged(false),
        _redrawPending(false),
        _redrawRequestListener(),
        _mapRendererListener(),
        _rendererCaptureListeners(),
        _rendererCaptureListenersMutex(),
        _onChangeListeners(),
        _onChangeListenersMutex(),
        _renderThreadCallbacks(),
        _renderThreadCallbacksMutex(),
        _mutex()
    {
    }
        
    MapRenderer::~MapRenderer() {
    }
        
    void MapRenderer::init() {
        _cullWorker->setComponents(shared_from_this(), _cullWorker);
        _cullThread = std::thread(std::ref(*_cullWorker));

        _billboardPlacementWorker->setComponents(shared_from_this(), _billboardPlacementWorker);
        _billboardPlacementThread = std::thread(std::ref(*_billboardPlacementWorker));
        
        _optionsListener = std::make_shared<OptionsListener>(shared_from_this());
        _options->registerOnChangeListener(_optionsListener);
    }

    void MapRenderer::deinit() {
        _options->unregisterOnChangeListener(_optionsListener);
        _optionsListener.reset();
        
        _cullWorker->stop();
        _cullThread.detach();
        
        _billboardPlacementWorker->stop();
        _billboardPlacementThread.detach();
    }
        
    std::shared_ptr<RedrawRequestListener> MapRenderer::getRedrawRequestListener() const {
         return _redrawRequestListener.get();
    }
        
    void MapRenderer::setRedrawRequestListener(const std::shared_ptr<RedrawRequestListener>& listener) {
        _redrawRequestListener.set(listener);
    }
        
    std::shared_ptr<MapRendererListener> MapRenderer::getMapRendererListener() const {
        return _mapRendererListener.get();
    }

    void MapRenderer::setMapRendererListener(const std::shared_ptr<MapRendererListener>& listener) {
        _mapRendererListener.set(listener);
    }

    ViewState MapRenderer::getViewState() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        ViewState viewState = _viewState;
        viewState.calculateViewState(*_options);
        return viewState;
    }

    std::shared_ptr<ProjectionSurface> MapRenderer::getProjectionSurface() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        std::shared_ptr<ProjectionSurface> projectionSurface = _viewState.getProjectionSurface();
        if (!projectionSurface) {
            projectionSurface = _options->getProjectionSurface();
        }
        return projectionSurface;
    }
        
    void MapRenderer::requestRedraw() const {
        DirectorPtr<RedrawRequestListener> redrawRequestListener = _redrawRequestListener;

        if (redrawRequestListener) {
            _redrawPending = true;
            redrawRequestListener->onRedrawRequested();
        }
    }
    
    void MapRenderer::captureRendering(const std::shared_ptr<RendererCaptureListener>& listener, bool waitWhileUpdating) {
        if (!listener) {
            throw NullArgumentException("Null listener");
        }

        {
            std::lock_guard<std::mutex> lock(_rendererCaptureListenersMutex);
            _rendererCaptureListeners.push_back(std::make_pair(DirectorPtr<RendererCaptureListener>(listener), waitWhileUpdating));
        }
        requestRedraw();
    }

    std::vector<std::shared_ptr<BillboardDrawData> > MapRenderer::getBillboardDrawDatas() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _billboardSorter.getSortedBillboardDrawDatas();
    }

    AnimationHandler& MapRenderer::getAnimationHandler() {
        return _animationHandler;
    }
    
    KineticEventHandler& MapRenderer::getKineticEventHandler() {
        return _kineticEventHandler;
    }
    
    void MapRenderer::calculateCameraEvent(CameraPanEvent& cameraEvent, float durationSeconds, bool updateKinetic) {
        if (durationSeconds > 0) {
            if (cameraEvent.isUseDelta()) {
                _animationHandler.setPanDelta(cameraEvent.getPosDelta(), durationSeconds);
            } else {
                _animationHandler.setPanTarget(cameraEvent.getPos(), durationSeconds);
            }
    
            // Animation will start on the next frame
            requestRedraw();
            return;
        }
    
        MapPos oldFocusPos;
        MapPos newFocusPos;
        float zoom;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            std::shared_ptr<ProjectionSurface> projectionSurface = getProjectionSurface();

            oldFocusPos = projectionSurface->calculateMapPos(_viewState.getFocusPos());
        
            // Calculate new focusPos, cameraPos and upVec
            cameraEvent.calculate(*_options, _viewState);
    
            // Calculate parameters for kinetic events
            newFocusPos = projectionSurface->calculateMapPos(_viewState.getFocusPos());
            zoom = _viewState.getZoom();
          
            // In case of seamless panning horizontal teleport, offset the delta focus pos
            oldFocusPos.setX(oldFocusPos.getX() + _viewState.getHorizontalLayerOffsetDir() * Const::WORLD_SIZE);
        }
    
        // Delay updating the layers, because view state will be updated only after onDrawFrame is called
        viewChanged(true);
    
        if (updateKinetic) {
            _kineticEventHandler.setPanDelta(std::make_pair(oldFocusPos, newFocusPos), zoom);
        } 
    }
        
    void MapRenderer::calculateCameraEvent(CameraRotationEvent& cameraEvent, float durationSeconds, bool updateKinetic) {
        if (durationSeconds > 0) {
            float oldRotation;
            {
                std::lock_guard<std::recursive_mutex> lock(_mutex);
                oldRotation = _viewState.getRotation();
            }
            _animationHandler.setRotationTarget(cameraEvent.isUseDelta() ? oldRotation + cameraEvent.getRotationDelta() : cameraEvent.getRotation(), cameraEvent.isUseTarget() ? &cameraEvent.getTargetPos() : nullptr, durationSeconds);
    
            // Animation will start on the next frame
            requestRedraw();
            return;
        }

        MapPos focusPos;
        float deltaRotation;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            float oldRotation = _viewState.getRotation();
            
            // Calculate new focusPos, cameraPos and upVec
            cameraEvent.calculate(*_options, _viewState);
            
            // Calculate parameters for kinetic events
            float rotation = _viewState.getRotation();
            deltaRotation = rotation - oldRotation;

            focusPos = getProjectionSurface()->calculateMapPos(_viewState.getFocusPos());
        }
    
        // Delay updating the layers, because view state will be updated only after onDrawFrame is called
        viewChanged(true);
        
        if (updateKinetic) {
            _kineticEventHandler.setRotationDelta(deltaRotation, cameraEvent.isUseTarget() ? cameraEvent.getTargetPos() : focusPos);
        }
    }
        
    void MapRenderer::calculateCameraEvent(CameraTiltEvent& cameraEvent, float durationSeconds, bool updateKinetic) {
        if (durationSeconds > 0) {
            float oldTilt;
            {
                std::lock_guard<std::recursive_mutex> lock(_mutex);
                oldTilt = _viewState.getTilt();
            }
            _animationHandler.setTiltTarget(cameraEvent.isUseDelta() ? oldTilt + cameraEvent.getTiltDelta() : cameraEvent.getTilt(), durationSeconds);
    
            // Animation will start on the next frame
            requestRedraw();
            return;
        }
    
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            // Calculate new focusPos, cameraPos and upVec
            cameraEvent.calculate(*_options, _viewState);
        }
    
        // Delay updating the layers, because view state will be updated only after onDrawFrame is called
        viewChanged(true);
    }
    
    void MapRenderer::calculateCameraEvent(CameraZoomEvent& cameraEvent, float durationSeconds, bool updateKinetic) {
        if (durationSeconds > 0) {
            float oldZoom;
            {
                std::lock_guard<std::recursive_mutex> lock(_mutex);
                oldZoom = _viewState.getZoom();
            }
            _animationHandler.setZoomTarget(cameraEvent.isUseDelta() ? oldZoom + cameraEvent.getZoomDelta() : cameraEvent.getZoom(), cameraEvent.isUseTarget() ? &cameraEvent.getTargetPos() : nullptr, durationSeconds);
    
            // Animation will start on the next frame
            requestRedraw();
            return;
        }
    
        MapPos focusPos;
        float deltaZoom;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            float oldZoom = _viewState.getZoom();
            
            // Calculate new focusPos, cameraPos and upVec
            cameraEvent.calculate(*_options, _viewState);
            
            // Calculate parameters for kinetic events
            float zoom = _viewState.getZoom();
            deltaZoom = zoom - oldZoom;

            focusPos = getProjectionSurface()->calculateMapPos(_viewState.getFocusPos());
        }
    
        // Delay updating the layers, because view state will be updated only after onDrawFrame is called
        viewChanged(true);
        
        if (updateKinetic) {
            _kineticEventHandler.setZoomDelta(deltaZoom, cameraEvent.isUseTarget() ? cameraEvent.getTargetPos() : focusPos);
        }
    }
    
    void MapRenderer::moveToFitBounds(const MapBounds& mapBounds, const ScreenBounds& screenBounds, bool integerZoom, bool resetTilt, bool resetRotation, float durationSeconds) {
        CameraPanEvent cameraPanEvent;
        CameraRotationEvent cameraRotationEvent;
        CameraTiltEvent cameraTiltEvent;
        CameraZoomEvent cameraZoomEvent;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            std::shared_ptr<ProjectionSurface> projectionSurface = getProjectionSurface();

            // Find center position
            cglib::vec3<double> centerPos(0, 0, 0);
            {
                cglib::vec3<double> minPos = projectionSurface->calculatePosition(mapBounds.getMin());
                cglib::vec3<double> maxPos = projectionSurface->calculatePosition(mapBounds.getMax());
                cglib::mat4x4<double> transform = projectionSurface->calculateTranslateMatrix(minPos, maxPos, 0.5);
                centerPos = cglib::transform_point(minPos, transform);
                if (std::isnan(cglib::norm(centerPos))) {
                    centerPos = cglib::vec3<double>(0, 0, 0);
                }
            }
            
            // Adjust the camera tilt, rotation and position to the final state of this animation
            cglib::vec3<double> focusPos = centerPos;
            cglib::vec3<double> oldFocusPos = _viewState.getFocusPos();
            cameraPanEvent.setPos(projectionSurface->calculateMapPos(centerPos));
            cameraPanEvent.calculate(*_options, _viewState);
            
            float rotation = 0;
            float oldRotation = _viewState.getRotation();
            if (resetRotation) {
                cameraRotationEvent.setRotation(0);
                cameraRotationEvent.calculate(*_options, _viewState);
            }
    
            float oldTilt = _viewState.getTilt();
            float tilt = 90;
            if (resetTilt) {
                cameraTiltEvent.setTilt(90);
                cameraTiltEvent.calculate(*_options, _viewState);
            }
            
            // Use binary search to determine what the zoom level of the final state should be, so that all the points
            // would fit in the view
            float oldZoom = _viewState.getZoom();
            MapRange zoomRange(_options->getZoomRange());
            float zoom = _options->getZoomRange().getMin();
            float zoomStep = zoomRange.length() * 0.5f;
            if (mapBounds.getMin() == mapBounds.getMax()) {
                zoom = oldZoom;
                zoomStep = 0;
            }

            // Hack: if view size is zero (view size not known), use given screen bounds for view dimensions
            ViewState viewState(_viewState);
            if (viewState.getWidth() == 0 && viewState.getHeight() == 0) {
                int width = static_cast<int>(screenBounds.getMax().getX() - screenBounds.getMin().getX());
                int height = static_cast<int>(screenBounds.getMax().getY() - screenBounds.getMin().getY());
                Log::Warnf("MapRenderer::moveToFitBounds: Screen size not known yet, using %d, %d", width, height);
                viewState.setScreenSize(width, height);
                viewState.calculateViewState(*_options);
            }

            for (int i = 0; i < 24; i++) {
                cameraZoomEvent.setZoom(zoom + zoomStep);
                cameraZoomEvent.calculate(*_options, viewState);
                viewState.clampZoom(*_options);

                ScreenPos screenPos = screenBounds.getCenter();
                cglib::vec3<double> pos = viewState.screenToWorld(cglib::vec2<float>(screenPos.getX(), screenPos.getY()), 0, _options);
                if (std::isnan(cglib::norm(pos))) {
                    Log::Error("MapRenderer::moveToFitBounds: Failed to translate screen position!");
                    return;
                }

                cglib::mat4x4<double> transform = projectionSurface->calculateTranslateMatrix(pos, focusPos, 1);
                focusPos = cglib::transform_point(centerPos, transform);
                cameraPanEvent.setPos(projectionSurface->calculateMapPos(focusPos));
                cameraPanEvent.calculate(*_options, viewState);
                viewState.clampFocusPos(*_options);
    
                bool fit = true;
                for (int j = 0; j < 4; j++) {
                    MapPos mapPos(j & 1 ? mapBounds.getMax().getX() : mapBounds.getMin().getX(), j & 2 ? mapBounds.getMax().getY() : mapBounds.getMin().getY());
                    cglib::vec2<float> screenPos = viewState.worldToScreen(projectionSurface->calculatePosition(mapPos), _options);
                    if (!screenBounds.contains(ScreenPos(screenPos(0), screenPos(1)))) {
                        fit = false;
                        break;
                    }
                    cglib::vec3<double> normal = projectionSurface->calculateNormal(mapPos);
                    if (cglib::dot_product(normal, _viewState.getCameraPos() - projectionSurface->calculatePosition(mapPos)) < 0) {
                        fit = false;
                        break;
                    }
                }
                if (fit) {
                    zoom += zoomStep;
                }
                zoomStep /= 2;
            }
            
            if (integerZoom) {
                zoom = (float) std::floor(zoom);
            }
            
            // Reset the camera position, rotation tilt and zoom to the starting state of this animation
            // And then animate them to the final state over time, if needed
            cameraPanEvent.setPos(projectionSurface->calculateMapPos(oldFocusPos));
            cameraPanEvent.calculate(*_options, _viewState);
            cameraPanEvent.setPos(projectionSurface->calculateMapPos(focusPos));
            
            if (resetRotation) {
                cameraRotationEvent.setRotation(oldRotation);
                cameraRotationEvent.calculate(*_options, _viewState);
                cameraRotationEvent.setTargetPos(projectionSurface->calculateMapPos(focusPos));
                cameraRotationEvent.setRotation(rotation);
            }
            
            if (resetTilt) {
                cameraTiltEvent.setTilt(oldTilt);
                cameraTiltEvent.calculate(*_options, _viewState);
                cameraTiltEvent.setTilt(tilt);
            }
            
            cameraZoomEvent.setZoom(oldZoom);
            cameraZoomEvent.calculate(*_options, _viewState);
            cameraZoomEvent.setTargetPos(projectionSurface->calculateMapPos(focusPos));
            cameraZoomEvent.setZoom(zoom);
        }
        
        // Animate the view
        calculateCameraEvent(cameraPanEvent, durationSeconds, false);
        if (resetRotation) {
            calculateCameraEvent(cameraRotationEvent, durationSeconds, false);
        }
        if (resetTilt) {
            calculateCameraEvent(cameraTiltEvent, durationSeconds, false);
        }
        calculateCameraEvent(cameraZoomEvent, durationSeconds, false);
    }
    
    void MapRenderer::onSurfaceCreated() {
        ThreadUtils::SetThreadPriority(ThreadPriority::MAXIMUM);
        
        GLContext::LoadExtensions();
    
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        _surfaceCreated = true;

        // Reset frame buffer manager
        if (_frameBufferManager) {
            _frameBufferManager->setGLThreadId(std::thread::id());
        }
        _frameBufferManager = std::make_shared<FrameBufferManager>();
        _frameBufferManager->setGLThreadId(std::this_thread::get_id());

        // Reset shader manager
        if (_shaderManager) {
            _shaderManager->setGLThreadId(std::thread::id());
        }
        _shaderManager = std::make_shared<ShaderManager>();
        _shaderManager->setGLThreadId(std::this_thread::get_id());

        // Reset texture manager
        if (_textureManager) {
            _textureManager->setGLThreadId(std::thread::id());
        }
        _textureManager = std::make_shared<TextureManager>();
        _textureManager->setGLThreadId(std::this_thread::get_id());

        // Reset style cache
        _styleCache = std::make_shared<StyleTextureCache>(_textureManager, STYLE_TEXTURE_CACHE_SIZE);

        // Reset screen blending state
        _currentBoundFBOs.clear();
        _screenFrameBuffer.reset();
        _screenBlendShader.reset();

        // Drop all thread callbacks, as context is invalidated
        {
            std::lock_guard<std::mutex> lock(_renderThreadCallbacksMutex);
            _renderThreadCallbacks.clear();
        }

        // Notify renderers about the event
        _backgroundRenderer.onSurfaceCreated(_shaderManager, _textureManager);
        _watermarkRenderer.onSurfaceCreated(_shaderManager, _textureManager);
    
        for (const std::shared_ptr<Layer>& layer : _layers->getAll()) {
            layer->onSurfaceCreated(_shaderManager, _textureManager);
        }
        
        GLContext::CheckGLError("MapRenderer::onSurfaceCreated");
    }
    
    void MapRenderer::onSurfaceChanged(int width, int height) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _viewState.setScreenSize(width, height);
        _viewState.calculateViewState(*_options);
        _viewState.clampZoom(*_options);
        _viewState.clampFocusPos(*_options);
        _screenFrameBuffer.reset(); // reset, as this depends on the surface dimensions
        _surfaceChanged = true;
    }
    
    void MapRenderer::onDrawFrame() {
        _redrawPending = false;

        std::vector<std::shared_ptr<OnChangeListener> > onChangeListeners;
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            onChangeListeners = _onChangeListeners;
        }

        DirectorPtr<MapRendererListener> mapRendererListener = _mapRendererListener;

        // Re-set GL thread ids, Windows Phone needs this as onSurfaceCreate/onSurfaceChange may be called from different threads
        _frameBufferManager->setGLThreadId(std::this_thread::get_id());
        _shaderManager->setGLThreadId(std::this_thread::get_id());
        _textureManager->setGLThreadId(std::this_thread::get_id());

        // Create pending textures and shaders
        _frameBufferManager->processFrameBuffers();
        _shaderManager->processShaders();
        _textureManager->processTextures();

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            if (_surfaceChanged) {
                glViewport(0, 0, _viewState.getWidth(), _viewState.getHeight());
                _watermarkRenderer.onSurfaceChanged(_viewState.getWidth(), _viewState.getHeight());
    
                _kineticEventHandler.stopPan();
                _kineticEventHandler.stopRotation();
                _kineticEventHandler.stopZoom();
            
                GLContext::CheckGLError("MapRenderer::onSurfaceChanged");
    
                _lastFrameTime = std::chrono::steady_clock::now();
            }
        }
        
        // Calculate time from the last frame
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        float deltaSeconds = std::chrono::duration_cast<std::chrono::duration<float> >(currentTime - _lastFrameTime).count();
        _lastFrameTime = currentTime;
    
        // Callback for synchronized rendering
        if (mapRendererListener) {
            mapRendererListener->onBeforeDrawFrame();
        }
        
        // Calculate camera params and make a synchronized copy of the view state
        ViewState viewState;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _viewState.calculateViewState(*_options);
            viewState = _viewState;
            _viewState.setHorizontalLayerOffsetDir(0);
        }

        // Don't delay calling the cull task, the view state was already updated
        if (_surfaceChanged.exchange(false)) {
            viewChanged(false);
        }
        
        // Calculate map moving animations and kinetic events
        _animationHandler.calculate(viewState, deltaSeconds);
        _kineticEventHandler.calculate(viewState, deltaSeconds);

        initializeRenderState();
    
        _backgroundRenderer.onDrawFrame(viewState);
        drawLayers(deltaSeconds, viewState);
        _watermarkRenderer.onDrawFrame(viewState);
    
        // Callback for synchronized rendering
        if (mapRendererListener) {
            mapRendererListener->onAfterDrawFrame();
        }

        // Update billboard placements/visibility
        if (_billboardsChanged.exchange(false)) {
            _billboardPlacementWorker->init(BILLBOARD_PLACEMENT_TASK_DELAY);
        }
        
        handleRenderThreadCallbacks();
        handleRendererCaptureCallbacks();
        
        // Call listener to inform we are idle now, if no redraw request is pending
        if (!_redrawPending) {
            for (const std::shared_ptr<OnChangeListener>& onChangeListener : onChangeListeners) {
                onChangeListener->onMapIdle();
            }
        }

        GLContext::CheckGLError("MapRenderer::onDrawFrame");
    }
    
    void MapRenderer::onSurfaceDestroyed() {
        // This method may never be called (e.x Android)
        _surfaceCreated = false;

        // Reset texture manager. We tell managers to ignore all resource 'release' operations by invalidating manager thread ids
        if (_textureManager) {
            _textureManager->setGLThreadId(std::thread::id());
            _textureManager.reset();
        }

        // Reset shader manager
        if (_shaderManager) {
            _shaderManager->setGLThreadId(std::thread::id());
            _shaderManager.reset();
        }

        // Reset frame buffer manager
        if (_frameBufferManager) {
            _frameBufferManager->setGLThreadId(std::thread::id());
            _frameBufferManager.reset();
        }

        // Reset style cache
        _styleCache.reset();

        // Reset screen blending state
        _currentBoundFBOs.clear();
        _screenFrameBuffer.reset();
        _screenBlendShader.reset();

        // Clean up all opengl resources
        for (const std::shared_ptr<Layer>& layer : _layers->getAll()) {
            layer->onSurfaceDestroyed();
        }
        
        _watermarkRenderer.onSurfaceDestroyed();
        _backgroundRenderer.onSurfaceDestroyed();

        // Drop all thread callbacks, as context is invalidated
        {
            std::lock_guard<std::mutex> lock(_renderThreadCallbacksMutex);
            _renderThreadCallbacks.clear();
        }
    }
    
    void MapRenderer::clearAndBindScreenFBO(const Color& color, bool depth, bool stencil) {
        GLint prevBoundFBO = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBoundFBO);
        GLuint bufferMask = GL_COLOR_BUFFER_BIT | (depth ? GL_DEPTH_BUFFER_BIT : 0) | (stencil ? GL_STENCIL_BUFFER_BIT : 0);
        _currentBoundFBOs.emplace_back(static_cast<GLuint>(prevBoundFBO), bufferMask);

        if (!_screenFrameBuffer) {
            _screenFrameBuffer = _frameBufferManager->createFrameBuffer(_viewState.getWidth(), _viewState.getHeight(), true, depth, stencil);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, _screenFrameBuffer->getFBOId());

        glClearColor(color.getR() / 255.0f, color.getG() / 255.0f, color.getB() / 255.0f, color.getA() / 255.0f);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        if (depth) {
            glDepthMask(GL_TRUE);
        }
        if (stencil) {
            glStencilMask(255);
        }

        glClear(bufferMask);

        if (depth) {
            glDepthMask(GL_FALSE);
        }
        if (stencil) {
            glStencilMask(0);
        }

        GLContext::CheckGLError("MapRenderer::clearAndBindScreenFBO");
    }

    void MapRenderer::blendAndUnbindScreenFBO(float opacity) {
        static const GLfloat screenVertices[8] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };

        if (_currentBoundFBOs.empty()) {
            Log::Error("MapRenderer::blendAndUnbindScreenFBO: No bound FBOs");
            return;
        }

        GLuint prevBoundFBO = _currentBoundFBOs.back().first;
        GLuint bufferMask = _currentBoundFBOs.back().second;
        _currentBoundFBOs.pop_back();
        
        if (!_screenFrameBuffer) {
            return; // should not happen, just safety
        }
        if (bufferMask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) {
            _screenFrameBuffer->discard(false, (bufferMask & GL_DEPTH_BUFFER_BIT) != 0, (bufferMask & GL_STENCIL_BUFFER_BIT) != 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, prevBoundFBO);

        if (!_screenBlendShader) {
            static const ShaderSource shaderSource("blend", &BLEND_VERTEX_SHADER, &BLEND_FRAGMENT_SHADER);
            
            _screenBlendShader = _shaderManager->createShader(shaderSource);
        }
        
        glUseProgram(_screenBlendShader->getProgId());

        glVertexAttribPointer(_screenBlendShader->getAttribLoc("a_coord"), 2, GL_FLOAT, GL_FALSE, 0, screenVertices);
        glEnableVertexAttribArray(_screenBlendShader->getAttribLoc("a_coord"));
        
        cglib::mat4x4<float> mvpMatrix = cglib::mat4x4<float>::identity();
        glUniformMatrix4fv(_screenBlendShader->getUniformLoc("u_mvpMat"), 1, GL_FALSE, mvpMatrix.data());
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _screenFrameBuffer->getColorTexId());
        glUniform1i(_screenBlendShader->getUniformLoc("u_tex"), 0);
        glUniform4f(_screenBlendShader->getUniformLoc("u_color"), opacity, opacity, opacity, opacity);
        glUniform2f(_screenBlendShader->getUniformLoc("u_invScreenSize"), 1.0f / _viewState.getWidth(), 1.0f / _viewState.getHeight());
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glDisableVertexAttribArray(_screenBlendShader->getAttribLoc("a_coord"));

        GLContext::CheckGLError("MapRenderer::blendAndUnbindScreenFBO");
    }

    void MapRenderer::setZBuffering(bool enable) {
        glDepthMask(enable ? GL_TRUE : GL_FALSE);
    }

    void MapRenderer::calculateRayIntersectedElements(const MapPos& targetPos, ViewState& viewState, std::vector<RayIntersectedElement>& results) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            viewState = _viewState;
        }
        if (!viewState.getProjectionSurface()) {
            return;
        }

        cglib::vec3<double> origin = viewState.getCameraPos();
        cglib::vec3<double> target = viewState.getProjectionSurface()->calculatePosition(targetPos);
        cglib::ray3<double> ray(origin, target - origin);
    
        // Normal layer click detection is done in the layer order
        for (const std::shared_ptr<Layer>& layer : _layers->getAll()) {
            layer->calculateRayIntersectedElements(ray, viewState, results);
        }
    }
     
    void MapRenderer::billboardsChanged() {
        _billboardsChanged = true;
    }
        
    void MapRenderer::layerChanged(const std::shared_ptr<Layer>& layer, bool delay) {
        // If screen size has been set, load the layers, otherwise wait for the onSurfaceChanged method
        // which will also start the cull worker
        if (_surfaceCreated) {
            // If layer not yet initialized, Force the layer to be initialized (and then culled) in the UI thread
            if (!layer->isSurfaceCreated()) {
                requestRedraw();
            } else {
                int delayTime = layer->getCullDelay();
                _cullWorker->init(layer, delay ? delayTime : 0);
            }
        }
    }
    
    void MapRenderer::viewChanged(bool delay) {
        for (const std::shared_ptr<Layer>& layer : _layers->getAll()) {
            int delayTime = layer->getCullDelay();
            _cullWorker->init(layer, delay ? delayTime : 0);
        }
    
        billboardsChanged();
    
        std::vector<std::shared_ptr<OnChangeListener> > onChangeListeners;
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            onChangeListeners = _onChangeListeners;
        }
        for (const std::shared_ptr<OnChangeListener>& onChangeListener : onChangeListeners) {
            onChangeListener->onMapChanged();
        }
        
        requestRedraw();
    }
    
    void MapRenderer::registerOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
        _onChangeListeners.push_back(listener);
    }

    void MapRenderer::unregisterOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
        _onChangeListeners.erase(std::remove(_onChangeListeners.begin(), _onChangeListeners.end(), listener), _onChangeListeners.end());
    }

    void MapRenderer::addRenderThreadCallback(const std::shared_ptr<ThreadWorker>& callback) {
        std::lock_guard<std::mutex> lock(_renderThreadCallbacksMutex);
        _renderThreadCallbacks.push_back(callback);
    }
    
    void MapRenderer::initializeRenderState() const {
        // Enable backface culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    
        // Enable blending, use premultiplied alpha
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
        // Disable dithering for better performance
        glDisable(GL_DITHER);
    
        // Enable depth testing, disable writing, set up clear color, etc
        Color clearColor = _options->getClearColor();
        glClearColor(clearColor.getR() / 255.0f, clearColor.getG() / 255.0f, clearColor.getB() / 255.0f, clearColor.getA() / 255.0f);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
        glDisable(GL_STENCIL_TEST);
        glStencilMask(255);
    
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glDepthMask(GL_FALSE);
        glStencilMask(0);
    }
    
    void MapRenderer::drawLayers(float deltaSeconds, const ViewState& viewState) {
        bool needRedraw = false;
        {
            std::vector<std::shared_ptr<Layer> > layers = _layers->getAll();

            // Reset surfaces if renderprojection has changed
            bool resetSurfaces = _renderProjectionChanged.exchange(false);

            // BillboardSorter modifications must be synchronized
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            // Clear billboard before sorting
            _billboardSorter.clear();

            // Do base drawing pass
            for (const std::shared_ptr<Layer>& layer : layers) {
                if (viewState.getHorizontalLayerOffsetDir() != 0) {
                    layer->offsetLayerHorizontally(viewState.getHorizontalLayerOffsetDir() * Const::WORLD_SIZE);
                }
    
                // Initialize layer renderer if it was added after onSurfaceCreated was called
                if (!layer->isSurfaceCreated() || resetSurfaces) {
                    if (layer->isSurfaceCreated()) {
                        layer->onSurfaceDestroyed();
                    }
                    layer->onSurfaceCreated(_shaderManager, _textureManager);
                    layerChanged(layer, false);
                }
    
                needRedraw = layer->onDrawFrame(deltaSeconds, _billboardSorter, *_styleCache, viewState) || needRedraw;
            }
            
            // Do 3D drawing pass
            for (const std::shared_ptr<Layer>& layer : layers) {
                needRedraw = layer->onDrawFrame3D(deltaSeconds, _billboardSorter, *_styleCache, viewState) || needRedraw;
            }
            
            // Sort billboards, calculate rotation state
            _billboardSorter.sort(viewState);
        }
        
        // Draw billboards, grouped by layer renderer
        if (!_billboardSorter.getSortedBillboardDrawDatas().empty()) {
            glDisable(GL_DEPTH_TEST);

            _billboardDrawDataBuffer.clear();
            std::shared_ptr<BillboardRenderer> prevRenderer;
            for (const std::shared_ptr<BillboardDrawData>& drawData : _billboardSorter.getSortedBillboardDrawDatas()) {
                if (std::shared_ptr<BillboardRenderer> renderer = drawData->getRenderer().lock()) {
                    if (prevRenderer && prevRenderer != renderer) {
                        prevRenderer->onDrawFrameSorted(deltaSeconds, _billboardDrawDataBuffer, *_styleCache, viewState);
                        _billboardDrawDataBuffer.clear();
                    }
            
                    _billboardDrawDataBuffer.push_back(drawData);
                    prevRenderer = renderer;
                }
            }
            if (prevRenderer) {
                prevRenderer->onDrawFrameSorted(deltaSeconds, _billboardDrawDataBuffer, *_styleCache, viewState);
            }

            glEnable(GL_DEPTH_TEST);
        }
    
        // Redraw, if needed
        if (needRedraw) {
            requestRedraw();
        }
    }
    
    void MapRenderer::handleRenderThreadCallbacks() {
        // Call all registered callbacks exacly once
        std::vector<std::shared_ptr<ThreadWorker> > renderThreadCallbacks;
        {
            std::lock_guard<std::mutex> lock(_renderThreadCallbacksMutex);
            std::swap(_renderThreadCallbacks, renderThreadCallbacks);
        }
        for (const std::shared_ptr<ThreadWorker>& callback : renderThreadCallbacks) {
            (*callback)();
        }
    }
    
    void MapRenderer::handleRendererCaptureCallbacks() {
        int width, height;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            width = _viewState.getWidth();
            height = _viewState.getHeight();
        }
        std::shared_ptr<Bitmap> captureBitmap;
        
        std::vector<std::pair<DirectorPtr<RendererCaptureListener>, bool> > rendererCaptureListeners;
        {
            std::lock_guard<std::mutex> lock(_rendererCaptureListenersMutex);
            _rendererCaptureListeners.swap(rendererCaptureListeners);
        }

        bool callbacksPending = false;
        for (std::size_t i = 0; i < rendererCaptureListeners.size(); i++) {
            const DirectorPtr<RendererCaptureListener>& listener = rendererCaptureListeners[i].first;
            bool waitWhileUpdating = rendererCaptureListeners[i].second;
            if (waitWhileUpdating) {
                bool layersUpdating = false;
                for (const std::shared_ptr<Layer>& layer : _layers->getAll()) {
                    if (layer->isUpdateInProgress()) {
                        layersUpdating = true;
                        break;
                    }
                }
                if (_redrawPending || layersUpdating || !_cullWorker->isIdle() || !_billboardPlacementWorker->isIdle()) {
                    std::lock_guard<std::mutex> lock(_rendererCaptureListenersMutex);
                    _rendererCaptureListeners.push_back(rendererCaptureListeners[i]);
                    callbacksPending = true;
                    continue;
                }
            }
            
            if (!captureBitmap) {
                std::vector<unsigned char> data(4 * width * height);
                glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
                captureBitmap = std::make_shared<Bitmap>(data.data(), width, height, ColorFormat::COLOR_FORMAT_RGBA, -4 * width);
            }
            
            listener->onMapRendered(captureBitmap);
        }
        if (callbacksPending) {
            requestRedraw();
        }
    }

    MapRenderer::OptionsListener::OptionsListener(const std::shared_ptr<MapRenderer>& mapRenderer) : _mapRenderer(mapRenderer)
    {
    }

    void MapRenderer::OptionsListener::onOptionChanged(const std::string& optionName) {
        if (auto mapRenderer = _mapRenderer.lock()) {
            bool updateView = false;

            if (optionName == "AmbientLightColor" || optionName == "MainLightColor" || optionName == "MainLightDirection" || optionName == "ClearColor" || optionName == "SkyColor") {
                updateView = true;
            }

            if (optionName.substr(0, 9) == "Watermark") {
                updateView = true;
            }

            if (optionName == "RenderProjectionMode" || optionName == "RenderProjection") {
                std::lock_guard<std::recursive_mutex> lock(mapRenderer->_mutex);
                mapRenderer->_viewState.calculateViewState(*mapRenderer->_options);
                mapRenderer->_renderProjectionChanged = true;
                updateView = true;
            }

            if (optionName == "BaseProjection") {
                std::lock_guard<std::recursive_mutex> lock(mapRenderer->_mutex);
                mapRenderer->_viewState.calculateViewState(*mapRenderer->_options);
                mapRenderer->_viewState.clampFocusPos(*mapRenderer->_options);
                updateView = true;
            }

            if (optionName == "TileDrawSize" || optionName == "DPI" || optionName == "DrawDistance" || optionName == "FieldOfViewY" || optionName == "FocusPointOffset") {
                std::lock_guard<std::recursive_mutex> lock(mapRenderer->_mutex);
                mapRenderer->_viewState.calculateViewState(*mapRenderer->_options);
                updateView = true;
            }

            if (optionName == "ZoomRange") {
                std::lock_guard<std::recursive_mutex> lock(mapRenderer->_mutex);
                mapRenderer->_viewState.calculateViewState(*mapRenderer->_options);
                mapRenderer->_viewState.clampZoom(*mapRenderer->_options);                
                updateView = true;
            }

            if (optionName == "PanBounds") {
                std::lock_guard<std::recursive_mutex> lock(mapRenderer->_mutex);
                mapRenderer->_viewState.calculateViewState(*mapRenderer->_options);
                mapRenderer->_viewState.clampFocusPos(*mapRenderer->_options);
                updateView = true;
            }

            if (optionName == "RestrictedPanning") {
                std::lock_guard<std::recursive_mutex> lock(mapRenderer->_mutex);
                mapRenderer->_viewState.calculateViewState(*mapRenderer->_options);
                mapRenderer->_viewState.clampZoom(*mapRenderer->_options);
                mapRenderer->_viewState.clampFocusPos(*mapRenderer->_options);
                updateView = true;
            }

            if (updateView) {
                mapRenderer->viewChanged(false);
            } else {
                mapRenderer->requestRedraw();
            }
        }
    }

    const int MapRenderer::BILLBOARD_PLACEMENT_TASK_DELAY = 200;

    const int MapRenderer::STYLE_TEXTURE_CACHE_SIZE = 8 * 1024 * 1024;

    const std::string MapRenderer::BLEND_VERTEX_SHADER = R"GLSL(
        #version 100
        attribute vec2 a_coord;
        uniform mat4 u_mvpMat;
        void main() {
            gl_Position = u_mvpMat * vec4(a_coord, 0.0, 1.0);
        }
    )GLSL";

    const std::string MapRenderer::BLEND_FRAGMENT_SHADER = R"GLSL(
        #version 100
        precision mediump float;
        uniform sampler2D u_tex;
        uniform lowp vec4 u_color;
        uniform mediump vec2 u_invScreenSize;
        void main() {
            vec4 texColor = texture2D(u_tex, gl_FragCoord.xy * u_invScreenSize);
            gl_FragColor = texColor * u_color;
        }
    )GLSL";
}
