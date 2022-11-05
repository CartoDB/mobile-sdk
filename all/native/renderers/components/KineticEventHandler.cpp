#include "KineticEventHandler.h"
#include "components/Options.h"
#include "core/MapPos.h"
#include "graphics/ViewState.h"
#include "projections/ProjectionSurface.h"
#include "renderers/MapRenderer.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

#include <numeric>

namespace carto {

    KineticEventHandler::KineticEventHandler(MapRenderer& mapRenderer, Options& options) :
        _pan(false),
        _panDelta(0),
        _panPositions(),
        _rotation(false),
        _rotationDelta(0),
        _rotationTargetPos(),
        _rotationDeltaSamples(AVERAGE_SAMPLE_COUNT + 1),
        _zoom(false),
        _zoomDelta(0),
        _zoomTargetPos(),
        _zoomDeltaSamples(AVERAGE_SAMPLE_COUNT + 1),
        _mapRenderer(mapRenderer),
        _options(options),
        _mutex()
    {
    }
    
    KineticEventHandler::~KineticEventHandler() {
    }
    
    void KineticEventHandler::calculate(const ViewState& viewState, float deltaSeconds) {
        std::optional<CameraPanEvent> cameraPanEvent;
        std::optional<CameraRotationEvent> cameraRotationEvent;
        std::optional<CameraZoomEvent> cameraZoomEvent;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            cameraPanEvent = calculatePan(viewState, deltaSeconds);
            cameraRotationEvent = calculateRotation(viewState, deltaSeconds);
            cameraZoomEvent = calculateZoom(viewState, deltaSeconds);
        }
        if (cameraPanEvent) {
            _mapRenderer.calculateCameraEvent(*cameraPanEvent, 0, false);
        }
        if (cameraRotationEvent) {
            _mapRenderer.calculateCameraEvent(*cameraRotationEvent, 0, false);
        }
        if (cameraZoomEvent) {
            _mapRenderer.calculateCameraEvent(*cameraZoomEvent, 0, false);
        }
    }
    
    bool KineticEventHandler::isPanning() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _pan;
    }
    
    void KineticEventHandler::setPanDelta(const std::pair<MapPos, MapPos>& panDelta, float zoom) {
        if (_options.isKineticPan()) {
            std::lock_guard<std::mutex> lock(_mutex);
    
            // Panning tolerance depends on the current zoom level
            std::shared_ptr<ProjectionSurface> projectionSurface = _mapRenderer.getProjectionSurface();
            long long twoPowZoom = static_cast<long long>(std::pow(2.0f, static_cast<int>(zoom)));
            cglib::vec3<double> pos0 = projectionSurface->calculatePosition(panDelta.first);
            cglib::vec3<double> pos1 = projectionSurface->calculatePosition(panDelta.second);
            _panDelta = static_cast<float>(projectionSurface->calculateDistance(pos0, pos1) / Const::WORLD_SIZE * twoPowZoom);
            if (_panDelta < KINETIC_PAN_START_TOLERANCE) {
                _panDelta = 0;
                return;
            }
            _panDelta = std::min(_panDelta * KINETIC_PAN_DELTA_MULTIPLIER, KINETIC_PAN_DELTA_CLAMP);
            _panPositions = panDelta;
        }
    }
    
    void KineticEventHandler::startPan() {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _pan = true;
        }
        _mapRenderer.requestRedraw();
    }
    
    void KineticEventHandler::stopPan() {
        std::lock_guard<std::mutex> lock(_mutex);
        _pan = false;
        _panDelta = 0;
    }
    
    bool KineticEventHandler::isRotating() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _rotation;
    }
    
    void KineticEventHandler::setRotationDelta(float rotationDelta, const MapPos& targetPos) {
        if (_options.isKineticRotation()) {
            std::lock_guard<std::mutex> lock(_mutex);
            
            if (rotationDelta > 180) {
                rotationDelta -= 360;
            } else if (rotationDelta <= -180) {
                rotationDelta += 360;
            }
        
            // Calculate the running deltaZoom average
            _rotationDeltaSamples.push_back(rotationDelta);
            if (_rotationDeltaSamples.size() > AVERAGE_SAMPLE_COUNT) {
                _rotationDeltaSamples.pop_front();
            }
            float average = std::accumulate(_rotationDeltaSamples.begin(), _rotationDeltaSamples.end(), 0.0f) / _rotationDeltaSamples.size();
    
            if (std::abs(average) < KINETIC_ROTATION_START_TOLERANCE_ANGLE) {
                _rotationDelta = 0;
                return;
            }
            _rotationDelta = std::min(std::abs(average) * KINETIC_ROTATION_DELTA_MULTIPLIER, KINETIC_ROTATION_DELTA_CLAMP) * (average < 0 ? -1 : 1);
            _rotationTargetPos = targetPos;
        }
    }
    
    void KineticEventHandler::startRotation() {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _rotation = true;
        }
        _mapRenderer.requestRedraw();
    }
    
    void KineticEventHandler::stopRotation() {
        std::lock_guard<std::mutex> lock(_mutex);
        _rotation = false;
        _rotationDelta = 0;
        _rotationDeltaSamples.clear();
    }
        
    bool KineticEventHandler::isZooming() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _zoom;
    }
    
    void KineticEventHandler::setZoomDelta(float deltaZoom, const MapPos& targetPos) {
        if (_options.isKineticZoom()) {
            
            // Calculate the running deltaZoom average
            _zoomDeltaSamples.push_back(deltaZoom);
            if (_zoomDeltaSamples.size() > AVERAGE_SAMPLE_COUNT) {
                _zoomDeltaSamples.pop_front();
            }
            float average = std::accumulate(_zoomDeltaSamples.begin(), _zoomDeltaSamples.end(), 0.0f) / _zoomDeltaSamples.size();
            
            if (std::abs(average) < KINETIC_ZOOM_START_TOLERANCE) {
                _zoomDelta = 0;
                return;
            }
            _zoomDelta = std::min(std::abs(average) * KINETIC_ZOOM_DELTA_MULTIPLIER, KINETIC_ZOOM_DELTA_CLAMP) * (average < 0 ? -1 : 1);
            _zoomTargetPos = targetPos;
        }
    }
    
    void KineticEventHandler::startZoom() {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _zoom = true;
        }
        _mapRenderer.requestRedraw();
    }
    
    void KineticEventHandler::stopZoom() {
        std::lock_guard<std::mutex> lock(_mutex);
        _zoom = false;
        _zoomDelta = 0;
        _zoomDeltaSamples.clear();
    }
    
    std::optional<CameraPanEvent> KineticEventHandler::calculatePan(const ViewState& viewState, float deltaSeconds) {
        if (_options.isKineticPan() && _pan) {
            if (_panDelta < KINETIC_PAN_STOP_TOLERANCE) {
                // Stop kinetic panning
                _pan = false;
                _panDelta = 0;
            } else {
                // Calculate delta time corrected position
                float factor = std::pow(1.0f - KINETIC_PAN_SLOWDOWN, deltaSeconds);
                _panDelta *= factor;
                std::shared_ptr<ProjectionSurface> projectionSurface = _mapRenderer.getProjectionSurface();
                cglib::vec3<double> pos0 = projectionSurface->calculatePosition(_panPositions.first);
                cglib::vec3<double> pos1 = projectionSurface->calculatePosition(_panPositions.second);
                cglib::mat4x4<double> transform = projectionSurface->calculateTranslateMatrix(pos0, pos1, _panDelta);
                MapPos newFocusPos = projectionSurface->calculateMapPos(cglib::transform_point(viewState.getFocusPos(), transform));
                CameraPanEvent cameraEvent;
                cameraEvent.setPos(newFocusPos);
                return cameraEvent;
            }
        }
        return std::optional<CameraPanEvent>();
    }
    
    std::optional<CameraRotationEvent> KineticEventHandler::calculateRotation(const ViewState& viewState, float deltaSeconds) {
        if (_options.isKineticRotation() && _rotation) {
            if (std::abs(_rotationDelta) < std::abs(KINETIC_ROTATION_STOP_TOLERANCE_ANGLE)) {
                // Stop kinetic rotation
                _rotation = false;
                _rotationDelta = 0;
                _rotationDeltaSamples.clear();
            } else {
                // Calculate delta time corrected position
                float factor = std::pow(1.0f - KINETIC_ROTATION_SLOWDOWN, deltaSeconds);
                float newRotation = -_rotationDelta * factor + viewState.getRotation() + _rotationDelta;
                _rotationDelta += viewState.getRotation() - newRotation;
                CameraRotationEvent cameraEvent;
                cameraEvent.setRotation(newRotation);
                cameraEvent.setTargetPos(_rotationTargetPos);
                return cameraEvent;
            }
        }
        return std::optional<CameraRotationEvent>();
    }
        
    std::optional<CameraZoomEvent> KineticEventHandler::calculateZoom(const ViewState& viewState, float deltaSeconds) {
        if (_options.isKineticZoom() && _zoom) {
            if (std::abs(_zoomDelta) < std::abs(KINETIC_ZOOM_STOP_TOLERANCE)) {
                // Stop kinetic zoom
                _zoom = false;
                _zoomDelta = 0;
                _zoomDeltaSamples.clear();
            } else {
                // Calculate delta time corrected position
                float factor = std::pow(1.0f - KINETIC_ZOOM_SLOWDOWN, deltaSeconds);
                float newZoom = -_zoomDelta * factor + viewState.getZoom() + _zoomDelta;
                _zoomDelta += viewState.getZoom() - newZoom;
                CameraZoomEvent cameraEvent;
                cameraEvent.setZoom(newZoom);
                cameraEvent.setTargetPos(_zoomTargetPos);
                return cameraEvent;
            }
        }
        return std::optional<CameraZoomEvent>();
    }
    
    const float KineticEventHandler::KINETIC_PAN_STOP_TOLERANCE = 0.007f;
    const float KineticEventHandler::KINETIC_PAN_START_TOLERANCE = 0.025f;
    const float KineticEventHandler::KINETIC_PAN_SLOWDOWN = 0.99f;
    const float KineticEventHandler::KINETIC_PAN_DELTA_MULTIPLIER = 7.0f;
    const float KineticEventHandler::KINETIC_PAN_DELTA_CLAMP = 1.0f;

    const float KineticEventHandler::KINETIC_ROTATION_STOP_TOLERANCE_ANGLE = 0.2f;
    const float KineticEventHandler::KINETIC_ROTATION_START_TOLERANCE_ANGLE = 1.0f;
    const float KineticEventHandler::KINETIC_ROTATION_SLOWDOWN = 0.99f;
    const float KineticEventHandler::KINETIC_ROTATION_DELTA_MULTIPLIER = 5.0f;
    const float KineticEventHandler::KINETIC_ROTATION_DELTA_CLAMP = 20.0f;

    const float KineticEventHandler::KINETIC_ZOOM_STOP_TOLERANCE = 0.05f;
    const float KineticEventHandler::KINETIC_ZOOM_START_TOLERANCE = 0.05f;
    const float KineticEventHandler::KINETIC_ZOOM_SLOWDOWN = 0.99f;
    const float KineticEventHandler::KINETIC_ZOOM_DELTA_MULTIPLIER = 5.0f;
    const float KineticEventHandler::KINETIC_ZOOM_DELTA_CLAMP = 1.0f;

    const unsigned int KineticEventHandler::AVERAGE_SAMPLE_COUNT = 7;

}
