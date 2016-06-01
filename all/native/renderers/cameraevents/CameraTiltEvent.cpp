#include "CameraTiltEvent.h"
#include "components/Options.h"
#include "graphics/ViewState.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

#include <cmath>

namespace carto {

    CameraTiltEvent::CameraTiltEvent() :
        _tilt(0),
        _tiltDelta(0),
        _useDelta(true)
    {
    }
    
    CameraTiltEvent::~CameraTiltEvent() {
    }
    
    float CameraTiltEvent::getTilt() const {
        return _tilt;
    }
    
    void CameraTiltEvent::setTilt(float tilt) {
        _tilt = tilt;
        _useDelta = false;
    }
    
    float CameraTiltEvent::getTiltDelta() const {
        return _tiltDelta;
    }
    
    void CameraTiltEvent::setTiltDelta(float tiltDelta) {
        _tiltDelta = tiltDelta;
        _useDelta = true;
    }
    
    bool CameraTiltEvent::isUseDelta() const {
        return _useDelta;
    }
    
    void CameraTiltEvent::calculate(Options& options, ViewState& viewState) {
        MapPos& cameraPos = viewState.getCameraPos();
        MapPos& focusPos = viewState.getFocusPos();
        MapVec& upVec = viewState.getUpVec();
    
        if (_useDelta) {
            // Calculate absolute tilt
            _tilt = viewState.getTilt() + _tiltDelta;
        }
    
        MapVec cameraVec = cameraPos - focusPos;
        double length = cameraVec.length();
    
        // Enforce tilt range
        MapRange tiltRange = options.getTiltRange();
        float tilt = GeneralUtils::Clamp(_tilt, tiltRange.getMin(), tiltRange.getMax());
    
        // Calculate camera parameters
        double sin = std::sin(tilt * Const::DEG_TO_RAD);
        cameraPos.setZ(sin * length);
        double lengthXY = std::sqrt(static_cast<long double>(
                std::max(0.0, length * length - cameraPos.getZ() * cameraPos.getZ())));
        cameraPos.setCoords(focusPos.getX() - upVec.getX() * lengthXY,
                focusPos.getY() - upVec.getY() * lengthXY);
    
        viewState.setTilt(tilt);
        
        // Calculate matrices etc. on the next onDrawFrame() call
        viewState.cameraChanged();
    }
    
}
