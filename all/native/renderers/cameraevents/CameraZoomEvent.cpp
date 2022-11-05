#include "CameraZoomEvent.h"
#include "CameraRotationEvent.h"
#include "components/Options.h"
#include "core/MapBounds.h"
#include "graphics/ViewState.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

namespace carto {

    CameraZoomEvent::CameraZoomEvent() :
        _keepRotation(false),
        _zoom(0.0f),
        _zoomDelta(0.0f),
        _targetPos(),
        _useDelta(true),
        _useTarget(false)
    {
    }
    
    CameraZoomEvent::~CameraZoomEvent() {
    }
    
    bool CameraZoomEvent::isKeepRotation() const {
        return _keepRotation;
    }

    void CameraZoomEvent::setKeepRotation(bool keepRotation) {
        _keepRotation = keepRotation;
    }

    float CameraZoomEvent::getZoom() const {
        return _zoom;
    }
    
    void CameraZoomEvent::setZoom(float zoom) {
        _zoom = zoom;
        _useDelta = false;
    }
    
    float CameraZoomEvent::getZoomDelta() const {
        return _zoomDelta;
    }
    
    void CameraZoomEvent::setZoomDelta(float zoomDelta) {
        _zoomDelta = zoomDelta;
        _useDelta = true;
    }
    
    void CameraZoomEvent::setScale(float scale) {
        _zoomDelta = static_cast<float>(std::log(1.0 / scale) / Const::LOGE_2);
        _useDelta = true;
    }
    
    const MapPos& CameraZoomEvent::getTargetPos() const {
        return _targetPos;
    }
    
    void CameraZoomEvent::setTargetPos(const MapPos& targetPos) {
        _targetPos = targetPos;
        _useTarget = true;
    }
    
    bool CameraZoomEvent::isUseDelta() const {
        return _useDelta;
    }
    
    bool CameraZoomEvent::isUseTarget() const {
        return _useTarget;
    }
    
    void CameraZoomEvent::calculate(Options& options, ViewState& viewState) {
        std::shared_ptr<ProjectionSurface> projectionSurface = viewState.getProjectionSurface();
        if (!projectionSurface) {
            return;
        }
        
        float rotation = viewState.getRotation();
        cglib::vec3<double> cameraPos = viewState.getCameraPos();
        cglib::vec3<double> focusPos = viewState.getFocusPos();
        cglib::vec3<double> upVec = viewState.getUpVec();

        if (!_useDelta) {
            _zoomDelta = _zoom - viewState.getZoom();
        }
    
        cglib::vec3<double> targetPos = focusPos;
        if (_useTarget) {
            targetPos = projectionSurface->calculatePosition(_targetPos);
        }
    
        MapRange zoomRange = options.getZoomRange();
        float zoom = GeneralUtils::Clamp(viewState.getZoom() + _zoomDelta, viewState.getMinZoom(), zoomRange.getMax());
        double scale = std::pow(2.0f, viewState.getZoom() - zoom);
        cglib::mat4x4<double> shiftTransform = projectionSurface->calculateTranslateMatrix(focusPos, targetPos, 1.0 - scale);

        focusPos = cglib::transform_point(focusPos, shiftTransform);
        cameraPos = focusPos + (cglib::transform_point(cameraPos, shiftTransform) - focusPos) * scale;
        upVec = cglib::transform_vector(upVec, shiftTransform);

        viewState.setCameraPos(cameraPos);
        viewState.setFocusPos(focusPos);
        viewState.setUpVec(upVec);
        viewState.setZoom(zoom);

        viewState.clampZoom(options);
        viewState.clampFocusPos(options);
        
        // Calculate matrices etc. on the next onDrawFrame() call
        viewState.cameraChanged();

        // Restore rotation, if needed
        if (_keepRotation) {
            CameraRotationEvent cameraRotationEvent;
            cameraRotationEvent.setRotation(rotation);
            cameraRotationEvent.calculate(options, viewState);
        }
    }
    
}
