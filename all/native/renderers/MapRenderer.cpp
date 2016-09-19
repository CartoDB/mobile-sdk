#include "MapRenderer.h"
#include "components/Exceptions.h"
#include "components/Layers.h"
#include "components/ThreadWorker.h"
#include "core/MapPos.h"
#include "core/ScreenPos.h"
#include "core/ScreenBounds.h"
#include "drawdatas/BillboardDrawData.h"
#include "graphics/Bitmap.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/TextureManager.h"
#include "graphics/utils/GLContext.h"
#include "layers/Layer.h"
#include "renderers/BillboardRenderer.h"
#include "renderers/MapRendererListener.h"
#include "renderers/RendererCaptureListener.h"
#include "renderers/RedrawRequestListener.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/cameraevents/CameraPanEvent.h"
#include "renderers/cameraevents/CameraRotationEvent.h"
#include "renderers/cameraevents/CameraTiltEvent.h"
#include "renderers/cameraevents/CameraZoomEvent.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/ThreadUtils.h"

namespace carto {

    MapRenderer::MapRenderer(const std::shared_ptr<Layers>& layers,
                             const std::shared_ptr<Options>& options) :
        _lastFrameTime(),
        _viewState(),
        _shaderManager(),
        _textureManager(),
        _styleCache(),
        _cullWorker(std::make_shared<CullWorker>()),
        _cullThread(),
        _backgroundRenderer(*options),
        _watermarkRenderer(*options),
        _billboardSorter(),
        _billboardDrawDataBuffer(),
        _billboardsChanged(false),
        _billboardPlacementWorker(std::make_shared<BillboardPlacementWorker>()),
        _billboardPlacementThread(),
        _animationHandler(*this),
        _kineticEventHandler(*this, *options),
        _layers(layers),
        _options(options),
        _surfaceChanged(false),
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
    
    MapPos MapRenderer::getCameraPos() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return MapPos(_viewState.getCameraPos());
    }
    
    MapPos MapRenderer::getFocusPos() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return MapPos(_viewState.getFocusPos());
    }
    
    MapVec MapRenderer::getUpVec() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return MapVec(_viewState.getUpVec());
    }
    
    float MapRenderer::getRotation() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _viewState.getRotation();
    }
    
    float MapRenderer::getTilt() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _viewState.getTilt();
    }
    
    float MapRenderer::getZoom() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _viewState.getZoom();
    }
    
    AnimationHandler& MapRenderer::getAnimationHandler() {
        return _animationHandler;
    }
    
    KineticEventHandler& MapRenderer::getKineticEventHandler() {
        return _kineticEventHandler;
    }
    
    void MapRenderer::calculateCameraEvent(CameraPanEvent& cameraEvent, float durationSeconds, bool updateKinetic) {
        if (durationSeconds > 0) {
            MapPos oldFocusPos;
            {
                std::lock_guard<std::recursive_mutex> lock(_mutex);
                oldFocusPos = _viewState.getFocusPos();
            }
            _animationHandler.setPanTarget(cameraEvent.isUseDelta() ? oldFocusPos + cameraEvent.getPosDelta() : cameraEvent.getPos(), durationSeconds);
    
            // Animation will start on the next frame
            requestRedraw();
            return;
        }
    
        MapVec deltaFocusPos;
        float zoom;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            MapPos oldFocusPos = _viewState.getFocusPos();
        
            // Calculate new focusPos, cameraPos and upVec
            cameraEvent.calculate(*_options, _viewState);
    
            // Calculate parameters for kinetic events
            MapPos focusPos = _viewState.getFocusPos();
            deltaFocusPos = focusPos - oldFocusPos;
            zoom = _viewState.getZoom();
          
            // In case of seamless panning horizontal teleport, offset the delta focus pos
            deltaFocusPos.setX(deltaFocusPos.getX() - _viewState.getHorizontalLayerOffsetDir() * Const::WORLD_SIZE);
        }
    
        // Delay updating the layers, because view state will be updated only after onDrawFrame is called
        viewChanged(true);
    
        if (updateKinetic) {
            _kineticEventHandler.setPanDelta(deltaFocusPos, zoom);
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
    
        float deltaRotation;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            float oldRotation = _viewState.getRotation();
            
            // Calculate new focusPos, cameraPos and upVec
            cameraEvent.calculate(*_options, _viewState);
            
            // Calculate parameters for kinetic events
            float rotation = _viewState.getRotation();
            deltaRotation = rotation - oldRotation;
        }
    
        // Delay updating the layers, because view state will be updated only after onDrawFrame is called
        viewChanged(true);
        
        if (updateKinetic) {
            _kineticEventHandler.setRotationDelta(deltaRotation, cameraEvent.getTargetPos());
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
    
        float deltaZoom;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            float oldZoom = _viewState.getZoom();
            
            // Calculate new focusPos, cameraPos and upVec
            cameraEvent.calculate(*_options, _viewState);
            
            // Calculate parameters for kinetic events
            float zoom = _viewState.getZoom();
            deltaZoom = zoom - oldZoom;
        }
    
        // Delay updating the layers, because view state will be updated only after onDrawFrame is called
        viewChanged(true);
        
        if (updateKinetic) {
            _kineticEventHandler.setZoomDelta(deltaZoom, cameraEvent.getTargetPos());
        }
    }
    
    void MapRenderer::moveToFitPoints(const MapPos& center, const std::vector<MapPos>& points, const ScreenBounds& screenBounds, bool integerZoom, bool resetTilt, bool resetRotation, float durationSeconds) {
        CameraPanEvent cameraPanEvent;
        CameraRotationEvent cameraRotationEvent;
        CameraTiltEvent cameraTiltEvent;
        CameraZoomEvent cameraZoomEvent;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            // Adjust the camera tilt, rotation and position to the final state of this animation
            MapPos focusPos = center;
            MapPos oldFocusPos = _viewState.getFocusPos();
            cameraPanEvent.setPos(center);
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

                MapVec delta = focusPos - viewState.screenToWorldPlane(screenBounds.getCenter(), _options);
                focusPos = center + delta;
                cameraPanEvent.setPos(focusPos);
                cameraPanEvent.calculate(*_options, viewState);
    
                bool fit = true;
                for (const MapPos& pos : points) {
                    ScreenPos screenPos = viewState.worldToScreen(pos, *_options);
                    if (!screenBounds.contains(screenPos)) {
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
            cameraPanEvent.setPos(oldFocusPos);
            cameraPanEvent.calculate(*_options, _viewState);
            cameraPanEvent.setPos(focusPos);
            
            if (resetRotation) {
                cameraRotationEvent.setRotation(oldRotation);
                cameraRotationEvent.calculate(*_options, _viewState);
                cameraRotationEvent.setTargetPos(focusPos);
                cameraRotationEvent.setRotation(rotation);
            }
            
            if (resetTilt) {
                cameraTiltEvent.setTilt(oldTilt);
                cameraTiltEvent.calculate(*_options, _viewState);
                cameraTiltEvent.setTilt(tilt);
            }
            
            cameraZoomEvent.setZoom(oldZoom);
            cameraZoomEvent.calculate(*_options, _viewState);
            cameraZoomEvent.setTargetPos(focusPos);
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
    
    MapPos MapRenderer::screenToWorld(const ScreenPos& screenPos) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _viewState.screenToWorldPlane(screenPos, _options);
    }
    
    ScreenPos MapRenderer::worldToScreen(const MapPos& worldPos) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _viewState.worldToScreen(worldPos, *_options);
    }
    
    void MapRenderer::onSurfaceCreated() {
        ThreadUtils::SetThreadPriority(ThreadPriority::MAXIMUM);
        
        GLContext::LoadExtensions();
    
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        _shaderManager = std::make_shared<ShaderManager>();
        _shaderManager->setGLThreadId(std::this_thread::get_id());
        _textureManager = std::make_shared<TextureManager>();
        _textureManager->setGLThreadId(std::this_thread::get_id());

        _styleCache = std::make_shared<StyleTextureCache>(_textureManager, STYLE_TEXTURE_CACHE_SIZE);

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
        _shaderManager->setGLThreadId(std::this_thread::get_id());
        _textureManager->setGLThreadId(std::this_thread::get_id());

        // Create pending textures and shaders
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

        if (_surfaceChanged) {
            _surfaceChanged = false;
            // Don't delay calling the cull task, the view state was already updated
            viewChanged(false);
        }
        
        // Calculate map moving animations and kinetic events
        _animationHandler.calculate(viewState, deltaSeconds);
        _kineticEventHandler.calculate(viewState, deltaSeconds);

        setUpGLState();
    
        _backgroundRenderer.onDrawFrame(viewState);
        drawLayers(deltaSeconds, viewState);
        _watermarkRenderer.onDrawFrame(viewState);
    
        // Callback for synchronized rendering
        if (mapRendererListener) {
            mapRendererListener->onAfterDrawFrame();
        }

        // Update billobard placements/visibility
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            if (_billboardsChanged) {
                _billboardsChanged = false;
                _billboardPlacementWorker->init(BILLBOARD_PLACEMENT_TASK_DELAY);
            }
        }
        
        handleRenderThreadCallbacks();
        handleRenderCaptureCallbacks();
        
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

        // Reset shader/texture manager. We tell managers to ignore all resource 'release' operations by invalidating manager thread ids
        if (_textureManager) {
            _textureManager->setGLThreadId(std::thread::id());
            _textureManager.reset();
        }
        if (_shaderManager) {
            _shaderManager->setGLThreadId(std::thread::id());
            _shaderManager.reset();
        }

        // Reset style cache
        _styleCache.reset();

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
    
    void MapRenderer::calculateRayIntersectedElements(const MapPos& targetPos, ViewState& viewState, std::vector<RayIntersectedElement>& results) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            viewState = _viewState;
        }

        MapPos rayOrigin = viewState.getCameraPos();
        MapVec rayDir = targetPos - viewState.getCameraPos();
        cglib::ray3<double> ray(cglib::vec3<double>(rayOrigin.getX(), rayOrigin.getY(), rayOrigin.getZ()), cglib::vec3<double>(rayDir.getX(), rayDir.getY(), rayDir.getZ()));
    
        // Normal layer click detection is done in the layer order
        const std::shared_ptr<Projection> projection = _options->getBaseProjection();
        for (const std::shared_ptr<Layer>& layer : _layers->getAll()) {
            layer->calculateRayIntersectedElements(*projection, ray, viewState, results);
        }
    
        // Sort the results
        auto distanceComparator = [&viewState](const RayIntersectedElement& element1, const RayIntersectedElement& element2) -> bool {
            if (element1.is3D() != element2.is3D()) {
                return element1.is3D() > element2.is3D();
            }
            if (element1.is3D()) {
                double deltaDistance = element1.getDistance(viewState.getCameraPos()) - element2.getDistance(viewState.getCameraPos());
                if (deltaDistance != 0) {
                    return deltaDistance < 0;
                }
            }
            return element1.getOrder() > element2.getOrder();
        };
        
        std::sort(results.begin(), results.end(), distanceComparator);
    }
     
    void MapRenderer::billboardsChanged() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _billboardsChanged = true;
    }
        
    void MapRenderer::layerChanged(const std::shared_ptr<Layer>& layer, bool delay) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        // If screen size has been set, load the layers, otherwise wait for the onSurfaceChanged method
        // which will also start the cull worker
        if (_viewState.getWidth() > 0 && _viewState.getHeight() > 0) {
            int delayTime = layer->getCullDelay();
            _cullWorker->init(layer, delay ? delayTime : 0);
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
    
    void MapRenderer::setUpGLState() const {
        // Set clear color
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
        // Enable backface culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    
        // Enable blending, use premultiplied alpha
        glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
        // Disable dithering for better performance
        glDisable(GL_DITHER);
    
        // Disable depth testing
        glDisable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
    
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    
    void MapRenderer::drawLayers(float deltaSeconds, const ViewState& viewState) {
        bool needRedraw = false;
        {
            std::vector<std::shared_ptr<Layer> > layers = _layers->getAll();

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
                if (!layer->isSurfaceCreated()) {
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
        BillboardRenderer* prevRenderer = NULL;
        _billboardDrawDataBuffer.clear();
        const std::vector<std::shared_ptr<BillboardDrawData> >& sortedBillboardDrawDatas = _billboardSorter.getSortedBillboardDrawDatas();
        for (const std::shared_ptr<BillboardDrawData>& drawData : sortedBillboardDrawDatas) {
            BillboardRenderer* renderer = drawData->getRenderer();
    
            if (prevRenderer && prevRenderer != renderer) {
                prevRenderer->onDrawFrameSorted(deltaSeconds, _billboardDrawDataBuffer, *_styleCache, viewState);
                _billboardDrawDataBuffer.clear();
            }
    
            _billboardDrawDataBuffer.push_back(drawData);
            prevRenderer = renderer;
        }
    
        if (prevRenderer) {
            prevRenderer->onDrawFrameSorted(deltaSeconds, _billboardDrawDataBuffer, *_styleCache, viewState);
        }
    
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
    
    void MapRenderer::handleRenderCaptureCallbacks() {
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
                    _rendererCaptureListeners.push_back(rendererCaptureListeners[i]);
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
    }
    
    MapRenderer::OptionsListener::OptionsListener(const std::shared_ptr<MapRenderer>& mapRenderer) : _mapRenderer(mapRenderer)
    {
    }

    void MapRenderer::OptionsListener::onOptionChanged(const std::string& optionName) {
        if (auto mapRenderer = _mapRenderer.lock()) {
            if (optionName == "ProjectionMode" || optionName == "TileDrawSize" || optionName == "DPI" || optionName == "DrawDistance" || optionName == "FieldOfViewY" || optionName == "FocusPointOffset") {
            	mapRenderer->viewChanged(false);
            }
            mapRenderer->requestRedraw();
        }
    }

    const int MapRenderer::BILLBOARD_PLACEMENT_TASK_DELAY = 200;

    const int MapRenderer::STYLE_TEXTURE_CACHE_SIZE = 8 * 1024 * 1024;

}
