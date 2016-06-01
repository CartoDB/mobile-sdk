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
        _useDelta = true;
    }
    
    bool CameraPanEvent::isUseDelta() const {
        return _useDelta;
    }
    
    void CameraPanEvent::calculate(Options& options, ViewState& viewState) {
        MapPos& cameraPos = viewState.getCameraPos();
        MapPos& focusPos = viewState.getFocusPos();
    
        if (_useDelta) {
            // If the object was initialized using relative coordinates
            // calculate the absolute focus position
            _pos = focusPos + _posDelta;
        }
    
        MapVec cameraVec = cameraPos - focusPos;
    
        // Clamp the focus pos to map bounds
        bool seamLess = options.isSeamlessPanning();
        MapBounds mapBounds = options.getInternalPanBounds();
        if (!seamLess || mapBounds.getMin().getX() >= -Const::HALF_WORLD_SIZE || mapBounds.getMax().getX() <= Const::HALF_WORLD_SIZE) {
            focusPos.setX(GeneralUtils::Clamp(_pos.getX(), mapBounds.getMin().getX(), mapBounds.getMax().getX()));
        }
        focusPos.setY(GeneralUtils::Clamp(_pos.getY(), mapBounds.getMin().getY(), mapBounds.getMax().getY()));
    
        // Teleport if necessary
        if (seamLess) {
          if (_pos.getX() > Const::HALF_WORLD_SIZE) {
            focusPos.setX(-Const::HALF_WORLD_SIZE + (_pos.getX() - Const::HALF_WORLD_SIZE));
            viewState.setHorizontalLayerOffsetDir(-1);
          } else if (_pos.getX() < -Const::HALF_WORLD_SIZE) {
            focusPos.setX(Const::HALF_WORLD_SIZE + (_pos.getX() + Const::HALF_WORLD_SIZE));
            viewState.setHorizontalLayerOffsetDir(1);
          }
        }
    
        cameraPos = focusPos;
        cameraPos += cameraVec;
        
        // Calculate matrices etc. on the next onDrawFrame() call
        viewState.cameraChanged();
    }
    
}
