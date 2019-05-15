#include "CameraPanEvent.h"
#include "CameraRotationEvent.h"
#include "components/Options.h"
#include "graphics/ViewState.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

namespace carto {

    CameraPanEvent::CameraPanEvent() :
        _keepRotation(false),
        _pos(),
        _posDelta(),
        _useDelta(true)
    {
    }
    
    CameraPanEvent::~CameraPanEvent() {
    }
    
    bool CameraPanEvent::isKeepRotation() const {
        return _keepRotation;
    }

    void CameraPanEvent::setKeepRotation(bool keepRotation) {
        _keepRotation = keepRotation;
    }

    const MapPos& CameraPanEvent::getPos() const {
        return _pos;
    }
    
    void CameraPanEvent::setPos(const MapPos& pos) {
        _pos = pos;
        _useDelta = false;
    }
    
    const std::pair<MapPos, MapPos>& CameraPanEvent::getPosDelta() const {
        return _posDelta;
    }
    
    void CameraPanEvent::setPosDelta(const std::pair<MapPos, MapPos>& posDelta) {
        _posDelta = posDelta;
        _useDelta = true;
    }
    
    bool CameraPanEvent::isUseDelta() const {
        return _useDelta;
    }
    
    void CameraPanEvent::calculate(Options& options, ViewState& viewState) {
        std::shared_ptr<ProjectionSurface> projectionSurface = viewState.getProjectionSurface();
        if (!projectionSurface) {
            return;
        }

        float rotation = viewState.getRotation();
        cglib::vec3<double> cameraPos = viewState.getCameraPos();
        cglib::vec3<double> focusPos = viewState.getFocusPos();
        cglib::vec3<double> upVec = viewState.getUpVec();

        cglib::mat4x4<double> translateTransform;
        if (_useDelta) {
            cglib::vec3<double> pos0 = projectionSurface->calculatePosition(_posDelta.first);
            cglib::vec3<double> pos1 = projectionSurface->calculatePosition(_posDelta.second);
            translateTransform = projectionSurface->calculateTranslateMatrix(pos0, pos1, 1.0f);
        } else {
            cglib::vec3<double> pos = projectionSurface->calculatePosition(_pos);
            translateTransform = projectionSurface->calculateTranslateMatrix(focusPos, pos, 1.0f);
        }

        focusPos = cglib::transform_point(focusPos, translateTransform);
        cameraPos = cglib::transform_point(cameraPos, translateTransform);
        upVec = cglib::transform_vector(upVec, translateTransform);
        
        viewState.setCameraPos(cameraPos);
        viewState.setFocusPos(focusPos);
        viewState.setUpVec(upVec);

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
