#include "CameraZoomEvent.h"
#include "components/Options.h"
#include "core/MapBounds.h"
#include "graphics/ViewState.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

namespace carto {

    CameraZoomEvent::CameraZoomEvent() :
        _zoom(0.0f),
        _zoomDelta(0.0f),
        _targetPos(),
        _useDelta(true),
        _useTarget(false)
    {
    }
    
    CameraZoomEvent::~CameraZoomEvent() {
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
        _targetPos.setZ(0);
        _useTarget = true;
    }
    
    bool CameraZoomEvent::isUseDelta() const {
        return _useDelta;
    }
    
    bool CameraZoomEvent::isUseTarget() const {
        return _useTarget;
    }
    
    void CameraZoomEvent::calculate(Options& options, ViewState& viewState) {
        MapPos cameraPos = viewState.getCameraPos();
        MapPos focusPos = viewState.getFocusPos();
    
        if (!_useDelta) {
            _zoomDelta = _zoom - viewState.getZoom();
        }
    
        if (!_useTarget) {
            _targetPos = focusPos;
        }
    
        MapRange zoomRange = options.getZoomRange();
        float zoom = GeneralUtils::Clamp(viewState.getZoom() + _zoomDelta, viewState.getMinZoom(), zoomRange.getMax());
        float scale = std::pow(2.0f, viewState.getZoom() - zoom);
    
        MapVec cameraVec(cameraPos - focusPos);
        cameraVec *= scale;
    
        MapVec targetVec(focusPos - _targetPos);
        targetVec *= scale;
        focusPos = _targetPos + targetVec;
        ClampFocusPos(focusPos, options);
        cameraPos = focusPos + cameraVec;

        viewState.setCameraPos(cameraPos);
        viewState.setFocusPos(focusPos);
        viewState.setZoom(zoom);

        viewState.clampFocusPos(options);
        
        // Calculate matrices etc. on the next onDrawFrame() call
        viewState.cameraChanged();
    }
    
}
