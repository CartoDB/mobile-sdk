#include "CameraRotationEvent.h"
#include "components/Options.h"
#include "graphics/ViewState.h"
#include "utils/Const.h"
#include "utils/GeneralUtils.h"

namespace carto {

    CameraRotationEvent::CameraRotationEvent() :
        _rotation(0),
        _sin(0),
        _cos(1),
        _targetPos(),
        _useDelta(true),
        _useTarget(false)
    {
    }
    
    CameraRotationEvent::~CameraRotationEvent() {
    }
    
    float CameraRotationEvent::getRotation() const {
        return _rotation;
    }
    
    void CameraRotationEvent::setRotation(float rotation) {
        _rotation = rotation;
        _useDelta = false;
    }
    
    float CameraRotationEvent::getRotationDelta() const {
        return static_cast<float>(std::atan2(_sin, _cos) * Const::RAD_TO_DEG);
    }
    
    void CameraRotationEvent::setRotationDelta(float rotationDelta) {
        _sin = std::sin(rotationDelta * Const::DEG_TO_RAD);
        _cos = std::cos(rotationDelta * Const::DEG_TO_RAD);
        _useDelta = true;
    }
    
    void CameraRotationEvent::setRotationDelta(double sin, double cos) {
        _sin = sin;
        _cos = cos;
        _useDelta = true;
    }
    
    const MapPos& CameraRotationEvent::getTargetPos() const {
        return _targetPos;
    }
    
    void CameraRotationEvent::setTargetPos(const MapPos& targetPos) {
        _targetPos = targetPos;
        _useTarget = true;
    }
    
    bool CameraRotationEvent::isUseDelta() const {
        return _useDelta;
    }
    
    bool CameraRotationEvent::isUseTarget() const {
        return _useTarget;
    }
    
    void CameraRotationEvent::calculate(Options& options, ViewState& viewState) {
        if (!options.isRotatable()) {
            return;
        }
        
        MapPos& cameraPos = viewState.getCameraPos();
        MapPos& focusPos = viewState.getFocusPos();
        MapVec& upVec = viewState.getUpVec();
        float rotation = viewState.getRotation();
    
        if (!_useDelta) {
            // If the object was initialized using absolute rotation, calculate delta rotation
            float rotationDelta = _rotation - viewState.getRotation();
            _sin = std::sin(rotationDelta * Const::DEG_TO_RAD);
            _cos = std::cos(rotationDelta * Const::DEG_TO_RAD);
            rotation = fmod(_rotation, 360.0f);
        } else {
            rotation += static_cast<float>(std::atan2(_sin, _cos) * Const::RAD_TO_DEG);
        }
    
        if (!_useTarget) {
            // If target was not specified rotate around the focus pos
            _targetPos = focusPos;
        }
        
        if (rotation > 180) {
            rotation -= 360;
        } else if (rotation <= -180) {
            rotation += 360;
        }
        
        viewState.setRotation(rotation);
    
        MapVec targetVec((cameraPos - _targetPos).rotate2D(_sin, _cos));
        cameraPos = _targetPos;
        cameraPos += targetVec;
    
        targetVec = (focusPos - _targetPos).rotate2D(_sin, _cos);
        focusPos = _targetPos;
        focusPos += targetVec;
    
        upVec.rotate2D(_sin, _cos);
        
        MapVec cameraVec = cameraPos - focusPos;
    
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
        
        // Calculate matrices etc. on the next onDrawFrame() call
        viewState.cameraChanged();
    }
    
}
