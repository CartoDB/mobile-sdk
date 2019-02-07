#include "CameraPanEvent.h"
#include "components/Options.h"
#include "graphics/ViewState.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

namespace carto {

    CameraPanEvent::CameraPanEvent() :
        _pos(),
        _posDelta(),
        _useDelta(true)
    {
    }
    
    CameraPanEvent::~CameraPanEvent() {
    }
    
    const MapPos& CameraPanEvent::getPos() const {
        return _pos;
    }
    
    void CameraPanEvent::setPos(const MapPos& pos) {
        _pos = pos;
        _useDelta = false;
    }
    
    const MapVec& CameraPanEvent::getPosDelta() const {
        return _posDelta;
    }
    
    void CameraPanEvent::setPosDelta(const MapVec& posDelta) {
        _posDelta = posDelta;
        _posDelta.setZ(0);
        _useDelta = true;
    }
    
    bool CameraPanEvent::isUseDelta() const {
        return _useDelta;
    }
    
    void CameraPanEvent::calculate(Options& options, ViewState& viewState) {
        MapPos cameraPos = viewState.getCameraPos();
        MapPos focusPos = viewState.getFocusPos();
    
        if (_useDelta) {
            // If the object was initialized using relative coordinates
            // calculate the absolute focus position
            _pos = focusPos + _posDelta;
        }
    
        MapVec cameraVec = cameraPos - focusPos;
        ClampFocusPos(focusPos, options);
        cameraPos = focusPos + cameraVec;

        viewState.setCameraPos(cameraPos);
        viewState.setFocusPos(focusPos);

        viewState.clampFocusPos(options);
        
        // Calculate matrices etc. on the next onDrawFrame() call
        viewState.cameraChanged();
    }
    
}
