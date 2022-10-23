#include "ui/MapView.h"
#include "ui/BaseMapView.h"
#include "ui/MapEventListener.h"
#include "ui/MapLicenseManagerListener.h"
#include "ui/MapRedrawRequestListener.h"
#include "ui/EmscriptenInput.h"
#include "components/Options.h"
#include "components/Layers.h"
#include "core/MapBounds.h"
#include "core/MapPos.h"
#include "core/MapVec.h"
#include "core/ScreenPos.h"
#include "core/ScreenBounds.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "renderers/MapRenderer.h"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>

#include <cmath>

namespace carto {
    std::vector<MapView*> cartoEmscriptenMapViews;
    bool cartoEmscriptenMainLoopStarted = false;
    bool waitResizeCount = 0;
    bool _runOnMainThread = true;

    void _emscripten_main_loop() {
        for (MapView* mapView : cartoEmscriptenMapViews) {
            if (mapView->isRenderPaused()) continue;
            if (mapView->needRedraw()) {
                mapView->onDrawFrame();
            }
        }
        if (waitResizeCount != 0) waitResizeCount -= 1;
    }

    EM_BOOL _emscripten_resize_callback(int eventType, const EmscriptenUiEvent *uiEvent, void *userData) {
            for (MapView* mapView : cartoEmscriptenMapViews) {
                mapView->onSurfaceChanged();
            }
        
        return true;
    }

    void *_thread_on_surface_created(void *arg) {
        bool runOnMainThread = MapView::getRunOnMainThread();
        MapView* mapView = (MapView*)arg;
        EmscriptenWebGLContextAttributes attr;
        emscripten_webgl_init_context_attributes(&attr);
        attr.explicitSwapControl = runOnMainThread ? 0 : 1;
        // attr.renderViaOffscreenBackBuffer = 1;
        // attr.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK;
        attr.alpha = 1;
        attr.depth = 1;
        attr.stencil = mapView->getStencil() ? 1 : 0;
        attr.antialias = 1;
        attr.preserveDrawingBuffer = 0;
        attr.failIfMajorPerformanceCaveat = 0;
        attr.premultipliedAlpha = 1;
        attr.enableExtensionsByDefault = 1;
        attr.majorVersion = 2;
        attr.minorVersion = 0;
        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context(mapView->getCanvasId().c_str(), &attr);
        emscripten_webgl_make_context_current(ctx);

        mapView->getBaseMapView()->onSurfaceCreated();
        mapView->onSurfaceChanged();

        if (!cartoEmscriptenMainLoopStarted) {
            cartoEmscriptenMainLoopStarted = true;
            emscripten_set_main_loop(_emscripten_main_loop, runOnMainThread ? 0 : 60, runOnMainThread ? 0 : 1);
        }

        return 0;
    }

    MapView::MapView(std::string canvasId, bool runOnMainThread, bool stencil) {
        this->setRunOnMainThread(runOnMainThread);
        _stencil = stencil;
        _canvasId = canvasId;
        _baseMapView = std::make_shared<BaseMapView>();
        _scale = emscripten_get_device_pixel_ratio();

        _baseMapView->getOptions()->setDPI(Const::UNSCALED_DPI * _scale);

        _redrawRequestListener = std::make_shared<MapRedrawRequestListener>(this);
        _baseMapView->setRedrawRequestListener(_redrawRequestListener);
    }

    MapView::~MapView() {
        cartoEmscriptenMapViews.erase(
            std::remove(cartoEmscriptenMapViews.begin(), cartoEmscriptenMapViews.end(), this),
            cartoEmscriptenMapViews.end()
        ); 
    }

    bool MapView::registerLicense(std::string licenseKey) {
        auto listener = std::make_shared<MapLicenseManagerListener>();
        return BaseMapView::RegisterLicense(licenseKey, listener);
    }

    void MapView::onSurfaceCreated() {
        emscripten_set_touchstart_callback(_canvasId.c_str(), this, true, &EmscriptenInput::_emscripten_touch_start);
        emscripten_set_touchmove_callback(_canvasId.c_str(), this, true, &EmscriptenInput::_emscripten_touch_move);
        emscripten_set_touchend_callback(_canvasId.c_str(), this, true, &EmscriptenInput::_emscripten_touch_end);
        emscripten_set_touchcancel_callback(_canvasId.c_str(), this, true, &EmscriptenInput::_emscripten_touch_cancel);

        emscripten_set_mousedown_callback(_canvasId.c_str(), this, true, &EmscriptenInput::_emscripten_mouse_start);
        emscripten_set_mousemove_callback(_canvasId.c_str(), this, true, &EmscriptenInput::_emscripten_mouse_move);
        emscripten_set_mouseup_callback(_canvasId.c_str(), this, true, &EmscriptenInput::_emscripten_mouse_end);
        emscripten_set_wheel_callback(_canvasId.c_str(), this, true, &EmscriptenInput::_emscripten_mouse_wheel);

        if (!std::count(cartoEmscriptenMapViews.begin(), cartoEmscriptenMapViews.end(), this)) {
            cartoEmscriptenMapViews.emplace_back(this);
        }

        emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, _emscripten_resize_callback);

        if (!MapView::getRunOnMainThread()) {
            pthread_attr_t attr;
            pthread_attr_init(&attr);

            emscripten_pthread_attr_settransferredcanvases(&attr, _canvasId.substr(1).c_str());

            pthread_t thread;
            pthread_create(&thread, &attr, _thread_on_surface_created, this);
        } else {
            _thread_on_surface_created(this);
        }
    }
    void MapView::onSurfaceChanged() {
        double canvasWidthDouble, canvasHeightDouble;
        emscripten_get_element_css_size(_canvasId.c_str(), &canvasWidthDouble, &canvasHeightDouble);
        int canvasWidthInt = (int)round(canvasWidthDouble);
        int canvasHeightInt = (int)round(canvasHeightDouble);
        if (_canvasWidth != canvasWidthInt || _canvasHeight != canvasHeightInt) {
            _canvasWidth = canvasWidthInt;
            _canvasHeight = canvasHeightInt;
            emscripten_set_canvas_element_size(_canvasId.c_str(), _canvasWidth, _canvasHeight);
            _baseMapView->onSurfaceChanged(_canvasWidth, _canvasHeight);
            _needRedraw = true;
        }
    }
    void MapView::onDrawFrame() {
        _baseMapView->onDrawFrame();
        // emscripten_webgl_commit_frame();
    }
    void MapView::requestRender() {
        _needRedraw = true;
    }
    void MapView::start() {
        this->onSurfaceCreated();
    }

    bool MapView::isRenderPaused() {
        return this->isPaused;
    }
    
    void MapView::setRenderPaused(bool isPaused) {
        this->isPaused = isPaused;
    }

    bool MapView::getStencil() {
        return _stencil;
    }

    void MapView::onInputEvent(int event, float x1, float y1, float x2, float y2) {
        _baseMapView->onInputEvent(event, x1, y1, x2, y2);
    }
    void MapView::onWheelEvent(int delta, float x, float y) {
        _baseMapView->onWheelEvent(delta, x, y);
    }

    float MapView::getScale() {
        return _scale;
    }

    int MapView::getCanvasWidth() {
        return _canvasWidth;
    }
    int MapView::getCanvasHeight() {
        return _canvasHeight;
    }

    std::string MapView::getCanvasId() {
        return _canvasId;
    }

    bool MapView::getRunOnMainThread() {
        return _runOnMainThread;
    }

    void MapView::setRunOnMainThread(bool status) {
        _runOnMainThread = status || !emscripten_supports_offscreencanvas();
        Log::Infof("runOnMainThread: %s", _runOnMainThread ? "true" : "false");
    }

    bool MapView::needRedraw() {
        return std::atomic_exchange(&_needRedraw, false);
    }

    std::shared_ptr<BaseMapView> MapView::getBaseMapView() {
        return _baseMapView;
    }

    const std::shared_ptr<Layers>& MapView::getLayers() const {
        return _baseMapView->getLayers();
    }
    const std::shared_ptr<Options>& MapView::getOptions() const {
        return _baseMapView->getOptions();
    }
    const std::shared_ptr<MapRenderer>& MapView::getMapRenderer() const {
        return _baseMapView->getMapRenderer();
    }
    MapPos MapView::getFocusPos() const {
        return _baseMapView->getFocusPos();
    }
    float MapView::getRotation() const {
        return _baseMapView->getRotation();
    }
    float MapView::getTilt() const {
        return _baseMapView->getTilt();
    }
    float MapView::getZoom() const {
        return _baseMapView->getZoom();
    }
    void MapView::pan(MapVec deltaPos, float durationSeconds) {
        _baseMapView->pan(deltaPos, durationSeconds);
    }
    void MapView::setFocusPos(MapPos pos, float durationSeconds) {
        _baseMapView->setFocusPos(pos, durationSeconds);
    }
    void MapView::rotate(float deltaAngle, float durationSeconds) {
        _baseMapView->rotate(deltaAngle, durationSeconds);
    }
    void MapView::rotate(float deltaAngle, MapPos targetPos, float durationSeconds) {
        _baseMapView->rotate(deltaAngle, targetPos, durationSeconds);
    }
    void MapView::setRotation(float angle, float durationSeconds) {
        _baseMapView->setRotation(angle, durationSeconds);
    }
    void MapView::setRotation(float angle, MapPos targetPos, float durationSeconds) {
        _baseMapView->setRotation(angle, targetPos, durationSeconds);
    }
    void MapView::tilt(float deltaTilt, float durationSeconds) {
        _baseMapView->tilt(deltaTilt, durationSeconds);
    }
    void MapView::setTilt(float tilt, float durationSeconds) {
        _baseMapView->setTilt(tilt, durationSeconds);
    }
    void MapView::zoom(float deltaZoom, float durationSeconds) {
        _baseMapView->zoom(deltaZoom, durationSeconds);
    }
    void MapView::zoom(float deltaZoom, MapPos targetPos, float durationSeconds) {
        _baseMapView->zoom(deltaZoom, targetPos, durationSeconds);
    }
    void MapView::setZoom(float zoom, float durationSeconds) {
        _baseMapView->setZoom(zoom, durationSeconds);
    }
    void MapView::setZoom(float zoom, MapPos targetPos, float durationSeconds) {
        _baseMapView->setZoom(zoom, targetPos, durationSeconds);
    }
    void MapView::moveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, bool integerZoom, float durationSeconds) {
        _baseMapView->moveToFitBounds(mapBounds, screenBounds, integerZoom, durationSeconds);
    }
    void MapView::moveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, bool integerZoom, bool resetRotation, 
        bool resetTilt, float durationSeconds) {
        _baseMapView->moveToFitBounds(mapBounds, screenBounds, integerZoom, resetRotation, resetTilt, durationSeconds);
    }
    std::shared_ptr<MapEventListener> MapView::getMapEventListener() const {
        return _baseMapView->getMapEventListener();
    }
    void MapView::setMapEventListener(const std::shared_ptr<MapEventListener>& mapEventListener) {
        _baseMapView->setMapEventListener(mapEventListener);
    }
    MapPos MapView::screenToMap(ScreenPos screenPos) {
        return _baseMapView->screenToMap(screenPos);
    }
    ScreenPos MapView::mapToScreen(MapPos mapPos) {
        return _baseMapView->mapToScreen(mapPos);
    }
    void MapView::cancelAllTasks() {
        _baseMapView->cancelAllTasks();
    }
    void MapView::clearPreloadingCaches() {
        _baseMapView->clearPreloadingCaches();
    }
    void MapView::clearAllCaches() {
        _baseMapView->clearAllCaches();
    }
}
