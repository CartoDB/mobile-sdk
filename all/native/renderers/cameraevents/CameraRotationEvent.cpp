#include "CameraRotationEvent.h"
#include "components/Options.h"
#include "graphics/ViewState.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "utils/Const.h"
#include "utils/Log.h"
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

        std::shared_ptr<ProjectionSurface> projectionSurface = viewState.getProjectionSurface();
        if (!projectionSurface) {
            return;
        }

        cglib::vec3<double> cameraPos = viewState.getCameraPos();
        cglib::vec3<double> focusPos = viewState.getFocusPos();
        cglib::vec3<double> upVec = viewState.getUpVec();
        float rotation = viewState.getRotation();
    
        if (!_useDelta) {
            // If the object was initialized using absolute rotation, calculate delta rotation
            float rotationDelta = _rotation - viewState.getRotation();
            _sin = std::sin(rotationDelta * Const::DEG_TO_RAD);
            _cos = std::cos(rotationDelta * Const::DEG_TO_RAD);
            rotation = std::fmod(_rotation, 360.0f);
        } else if (!(_sin == 0 && _cos == 0)) {
            rotation += static_cast<float>(std::atan2(_sin, _cos) * Const::RAD_TO_DEG);
        }
    
        cglib::vec3<double> targetPos = focusPos;
        if (_useTarget) {
            // Use specified target instead of focus position, if specified
            targetPos = projectionSurface->calculatePosition(_targetPos);
        }
        
        if (rotation > 180) {
            rotation -= 360;
        } else if (rotation <= -180) {
            rotation += 360;
        }
        
        cglib::vec3<double> axis = projectionSurface->calculateNormal(projectionSurface->calculateMapPos(targetPos));
        if (cglib::length(axis) == 0 || (_sin == 0 && _cos == 0)) {
            return;
        }
        double angle = std::atan2(_sin, _cos);
        cglib::mat4x4<double> rotateTransform = cglib::translate4_matrix(targetPos) * cglib::rotate4_matrix(axis, angle) * cglib::translate4_matrix(-targetPos);
        
        focusPos = cglib::transform_point(focusPos, rotateTransform);
        cameraPos = cglib::transform_point(cameraPos, rotateTransform);
        upVec = cglib::transform_vector(upVec, rotateTransform);
        
        viewState.setCameraPos(cameraPos);
        viewState.setFocusPos(focusPos);
        viewState.setUpVec(upVec);
        viewState.setRotation(rotation);

        viewState.clampFocusPos(options);

        // Calculate matrices etc. on the next onDrawFrame() call
        viewState.cameraChanged();
    }
    
}
