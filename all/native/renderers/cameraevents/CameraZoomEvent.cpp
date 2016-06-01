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
        _useTarget = true;
    }
    
    bool CameraZoomEvent::isUseDelta() const {
        return _useDelta;
    }
    
    bool CameraZoomEvent::isUseTarget() const {
        return _useTarget;
    }
    
    void CameraZoomEvent::calculate(Options& options, ViewState& viewState) {
        MapPos& cameraPos = viewState.getCameraPos();
        MapPos& focusPos = viewState.getFocusPos();
    
        if (!_useDelta) {
            _zoomDelta = _zoom - viewState.getZoom();
        }
    
        if (!_useTarget) {
            _targetPos = focusPos;
        }
    
        MapRange zoomRange = options.getZoomRange();
        float zoom = GeneralUtils::Clamp(viewState.getZoom() + _zoomDelta, zoomRange.getMin(), zoomRange.getMax());
        float scale = std::pow(2.0f, viewState.getZoom() - zoom);
    
        MapVec cameraVec(cameraPos - focusPos);
        cameraVec *= scale;
    
        MapVec targetVec(focusPos - _targetPos);
        targetVec *= scale;
        focusPos = _targetPos;
        focusPos += targetVec;
    
        // Enforce map bounds
        MapBounds mapBounds = options.getInternalPanBounds();
        bool seamLess = options.isSeamlessPanning();
        if (!seamLess || mapBounds.getMin().getX() >= -Const::HALF_WORLD_SIZE || mapBounds.getMax().getX() <= Const::HALF_WORLD_SIZE) {
            focusPos.setX(GeneralUtils::Clamp(focusPos.getX(), mapBounds.getMin().getX(), mapBounds.getMax().getX()));
        }
        focusPos.setY(GeneralUtils::Clamp(focusPos.getY(), mapBounds.getMin().getY(), mapBounds.getMax().getY()));
    
        // Teleport if necessary
        if (seamLess) {
          if (focusPos.getX() > Const::HALF_WORLD_SIZE) {
            focusPos.setX(-Const::HALF_WORLD_SIZE + (focusPos.getX() - Const::HALF_WORLD_SIZE));
            viewState.setHorizontalLayerOffsetDir(-1);
          } else if (focusPos.getX() < -Const::HALF_WORLD_SIZE) {
            focusPos.setX(Const::HALF_WORLD_SIZE + (focusPos.getX() + Const::HALF_WORLD_SIZE));
            viewState.setHorizontalLayerOffsetDir(1);
          }
        }
    
        cameraPos = focusPos;
        cameraPos += cameraVec;
    
        viewState.setZoom(zoom);
        
        // Calculate matrices etc. on the next onDrawFrame() call
        viewState.cameraChanged();
    }
    
}
