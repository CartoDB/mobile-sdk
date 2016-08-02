#include "BaseMapView.h"
#include "components/CancelableThreadPool.h"
#include "components/LicenseManager.h"
#include "components/Layers.h"
#include "core/MapPos.h"
#include "core/MapBounds.h"
#include "core/ScreenPos.h"
#include "core/ScreenBounds.h"
#include "layers/Layer.h"
#include "layers/TileLayer.h"
#include "projections/Projection.h"
#include "renderers/MapRenderer.h"
#include "renderers/cameraevents/CameraPanEvent.h"
#include "renderers/cameraevents/CameraRotationEvent.h"
#include "renderers/cameraevents/CameraTiltEvent.h"
#include "renderers/cameraevents/CameraZoomEvent.h"
#include "renderers/cameraevents/CameraPanEvent.h"
#include "ui/TouchHandler.h"
#include "utils/PlatformUtils.h"
#include "utils/Log.h"

#include <list>
#include <unordered_map>
#include <vector>
#include <sstream>

namespace carto {

    bool BaseMapView::RegisterLicense(const std::string& licenseKey, const std::shared_ptr<LicenseManagerListener>& listener) {
        return LicenseManager::GetInstance().registerLicense(licenseKey, listener);
    }

    std::string BaseMapView::GetSDKVersion() {
        std::string platformId = PlatformUtils::GetPlatformId();
        std::string deviceOS = PlatformUtils::GetDeviceOS();
        std::string deviceType = PlatformUtils::GetDeviceType();

        std::stringstream ss;
        ss << "Build: " << platformId << "-" << _CARTO_MOBILE_SDK_VERSION;
        ss << ", time: " << __DATE__ << " " << __TIME__;
        ss << ", device type: " << deviceType;
        ss << ", device OS: " << deviceOS;
        return ss.str();
    }
    
    BaseMapView::BaseMapView() :
        _envelopeThreadPool(std::make_shared<CancelableThreadPool>()),
        _tileThreadPool(std::make_shared<CancelableThreadPool>()),
        _options(std::make_shared<Options>(_envelopeThreadPool, _tileThreadPool)),
        _layers(std::make_shared<Layers>(_envelopeThreadPool, _tileThreadPool, _options)),
        _mapRenderer(std::make_shared<MapRenderer>(_layers, _options)),
        _touchHandler(std::make_shared<TouchHandler>(_mapRenderer, _options)),
        _mutex()
    {
        _mapRenderer->init();
        _touchHandler->init();
        _layers->setComponents(_mapRenderer, _touchHandler);
        
        setFocusPos(MapPos(), 0);
        setRotation(0, 0);
        setTilt(90, 0);
        setZoom(0, 0);

        Log::Infof("BaseMapView: %s", GetSDKVersion().c_str());
    }
    
    BaseMapView::~BaseMapView() {
        // Set stop flag and detach every thread, once the thread quits
        // all objects they hold will be released
        _envelopeThreadPool->deinit();
        _tileThreadPool->deinit();
        _mapRenderer->deinit();
        _touchHandler->deinit();
    }
    
    void BaseMapView::onSurfaceCreated() {
        Log::Info("BaseMapView::onSurfaceCreated()");
        _mapRenderer->onSurfaceCreated();
    }
    
    void BaseMapView::onSurfaceChanged(int width, int height) {
        Log::Infof("BaseMapView::onSurfaceChanged(): width: %d, height: %d", width, height);
        _mapRenderer->onSurfaceChanged(width, height);
    }
    
    void BaseMapView::onDrawFrame() {
        _mapRenderer->onDrawFrame();
    }
    
    void BaseMapView::onSurfaceDestroyed() {
        Log::Info("BaseMapView::onSurfaceDestroyed()");
        _mapRenderer->onSurfaceDestroyed();
    }
    
    void BaseMapView::onInputEvent(int event, float x1, float y1, float x2, float y2) {
        _touchHandler->onTouchEvent(event, ScreenPos(x1, y1), ScreenPos(x2, y2));
    }
    
    MapPos BaseMapView::getFocusPos() const {
        return _options->getBaseProjection()->fromInternal(_mapRenderer->getFocusPos());
    }
    
    float BaseMapView::getRotation() const {
        return _mapRenderer->getRotation();
    }
    
    float BaseMapView::getTilt() const {
        return _mapRenderer->getTilt();
    }
    
    float BaseMapView::getZoom() const {
        return _mapRenderer->getZoom();
    }
    
    void BaseMapView::pan(const MapVec& deltaPos, float durationSeconds) {
        MapPos focusPos(getFocusPos());
        focusPos += deltaPos;
        setFocusPos(focusPos, durationSeconds);
    }
    
    void BaseMapView::setFocusPos(const MapPos& pos, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopPan();
        _mapRenderer->getKineticEventHandler().stopPan();
        
        CameraPanEvent cameraEvent;
        cameraEvent.setPos(_options->getBaseProjection()->toInternal(pos));
        _mapRenderer->calculateCameraEvent(cameraEvent, durationSeconds, false);
    }
    
    void BaseMapView::rotate(float rotationDelta, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopRotation();
        _mapRenderer->getKineticEventHandler().stopRotation();
        
        CameraRotationEvent cameraEvent;
        cameraEvent.setRotationDelta(rotationDelta);
        _mapRenderer->calculateCameraEvent(cameraEvent, durationSeconds, false);
    }
        
    void BaseMapView::rotate(float rotationDelta, const MapPos& targetPos, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopRotation();
        _mapRenderer->getKineticEventHandler().stopRotation();
        
        CameraRotationEvent cameraEvent;
        cameraEvent.setRotationDelta(rotationDelta);
        cameraEvent.setTargetPos(_options->getBaseProjection()->toInternal(targetPos));
        _mapRenderer->calculateCameraEvent(cameraEvent, durationSeconds, false);
    }
    
    void BaseMapView::setRotation(float rotation, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopRotation();
        _mapRenderer->getKineticEventHandler().stopRotation();
        
        CameraRotationEvent cameraEvent;
        cameraEvent.setRotation(rotation);
        _mapRenderer->calculateCameraEvent(cameraEvent, durationSeconds, false);
    }
        
    void BaseMapView::setRotation(float rotation, const MapPos& targetPos, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopRotation();
        _mapRenderer->getKineticEventHandler().stopRotation();
        
        CameraRotationEvent cameraEvent;
        cameraEvent.setRotation(rotation);
        cameraEvent.setTargetPos(_options->getBaseProjection()->toInternal(targetPos));
        _mapRenderer->calculateCameraEvent(cameraEvent, durationSeconds, false);
    }
    
    void BaseMapView::tilt(float tiltDelta, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopTilt();
        
        CameraTiltEvent cameraEvent;
        cameraEvent.setTiltDelta(tiltDelta);
        _mapRenderer->calculateCameraEvent(cameraEvent, durationSeconds, false);
    }
    
    void BaseMapView::setTilt(float tilt, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopTilt();
        
        CameraTiltEvent cameraEvent;
        cameraEvent.setTilt(tilt);
        _mapRenderer->calculateCameraEvent(cameraEvent, durationSeconds, false);
    }
    
    void BaseMapView::zoom(float zoomDelta, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopZoom();
        _mapRenderer->getKineticEventHandler().stopZoom();
        
        CameraZoomEvent cameraEvent;
        cameraEvent.setZoomDelta(zoomDelta);
        _mapRenderer->calculateCameraEvent(cameraEvent, durationSeconds, false);
    }
        
    void BaseMapView::zoom(float zoomDelta, const MapPos& targetPos, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopZoom();
        _mapRenderer->getKineticEventHandler().stopZoom();
        
        CameraZoomEvent cameraEvent;
        cameraEvent.setZoomDelta(zoomDelta);
        cameraEvent.setTargetPos(_options->getBaseProjection()->toInternal(targetPos));
        _mapRenderer->calculateCameraEvent(cameraEvent, durationSeconds, false);
    }
    
    void BaseMapView::setZoom(float zoom, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopZoom();
        _mapRenderer->getKineticEventHandler().stopZoom();
        
        CameraZoomEvent cameraEvent;
        cameraEvent.setZoom(zoom);
        _mapRenderer->calculateCameraEvent(cameraEvent, durationSeconds, false);
    }
        
    void BaseMapView::setZoom(float zoom, const MapPos& targetPos, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopZoom();
        _mapRenderer->getKineticEventHandler().stopZoom();
        
        CameraZoomEvent cameraEvent;
        cameraEvent.setZoom(zoom);
        cameraEvent.setTargetPos(_options->getBaseProjection()->toInternal(targetPos));
        _mapRenderer->calculateCameraEvent(cameraEvent, durationSeconds, false);
    }
        
    void BaseMapView::moveToFitBounds(const MapBounds& mapBounds, const ScreenBounds& screenBounds, bool integerZoom, float durationSeconds) {
        moveToFitBounds(mapBounds, screenBounds, integerZoom, false, false, durationSeconds);
    }
        
    void BaseMapView::moveToFitBounds(const MapBounds& mapBounds, const ScreenBounds& screenBounds, bool integerZoom, bool resetRotation, bool resetTilt, float durationSeconds) {
        _mapRenderer->getAnimationHandler().stopZoom();
        _mapRenderer->getKineticEventHandler().stopZoom();
        
        MapBounds internalBounds(_options->getBaseProjection()->toInternal(mapBounds.getMin()), _options->getBaseProjection()->toInternal(mapBounds.getMax()));
        std::vector<MapPos> internalCorners;
        internalCorners.reserve(4);
        internalCorners.emplace_back(internalBounds.getMin());
        internalCorners.emplace_back(internalBounds.getMin().getX(), internalBounds.getMax().getY());
        internalCorners.emplace_back(internalBounds.getMax());
        internalCorners.emplace_back(internalBounds.getMax().getX(), internalBounds.getMin().getY());
        _mapRenderer->moveToFitPoints(internalBounds.getCenter(), internalCorners, screenBounds, integerZoom, resetTilt, resetRotation, durationSeconds);
    }
    
    std::shared_ptr<MapEventListener> BaseMapView::getMapEventListener() const {
        return _touchHandler->getMapEventListener();
    }
        
    void BaseMapView::setMapEventListener(const std::shared_ptr<MapEventListener>& mapEventListener) {
        _touchHandler->setMapEventListener(mapEventListener);
    }
        
    std::shared_ptr<RedrawRequestListener> BaseMapView::getRedrawRequestListener() const{
        return _mapRenderer->getRedrawRequestListener();
    }
        
    void BaseMapView::setRedrawRequestListener(const std::shared_ptr<RedrawRequestListener>& listener) {
        _mapRenderer->setRedrawRequestListener(listener);
    }
        
    MapPos BaseMapView::screenToMap(const ScreenPos& screenPos) {
        return _options->getBaseProjection()->fromInternal(_mapRenderer->screenToWorld(screenPos));
    }
    
    ScreenPos BaseMapView::mapToScreen(const MapPos& mapPos) {
        return _mapRenderer->worldToScreen(_options->getBaseProjection()->toInternal(mapPos));
    }
    
    void BaseMapView::cancelAllTasks() {
        _envelopeThreadPool->cancelAll();
        _tileThreadPool->cancelAll();
    }
    
    void BaseMapView::clearPreloadingCaches() {
        for (const std::shared_ptr<Layer>& layer : _layers->getAll()) {
            if (const std::shared_ptr<TileLayer>& tileLayer = std::dynamic_pointer_cast<TileLayer>(layer)) {
                tileLayer->clearTileCaches(false);
            }
        }
    }
    
    void BaseMapView::clearAllCaches() {
        for (const std::shared_ptr<Layer>& layer : _layers->getAll()) {
            if (const std::shared_ptr<TileLayer>& tileLayer = std::dynamic_pointer_cast<TileLayer>(layer)) {
                tileLayer->clearTileCaches(true);
            }
        }
    }
    
    const std::shared_ptr<Layers>& BaseMapView::getLayers() const {
        return _layers;
    }
    
    const std::shared_ptr<Options>& BaseMapView::getOptions() const {
        return _options;
    }

    const std::shared_ptr<MapRenderer>& BaseMapView::getMapRenderer() const {
        return _mapRenderer;
    }
        
}
