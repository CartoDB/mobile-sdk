#include "MapInteractionInfo.h"

namespace carto {

    MapInteractionInfo::MapInteractionInfo(bool panAction, bool zoomAction, bool rotateAction, bool tiltAction, bool animationStarted) :
        _panAction(panAction),
        _zoomAction(zoomAction),
        _rotateAction(rotateAction),
        _tiltAction(tiltAction),
        _animationStarted(animationStarted)
    {
    }

    MapInteractionInfo::~MapInteractionInfo() {
    }

    bool MapInteractionInfo::isPanAction() const {
        return _panAction;
    }

    bool MapInteractionInfo::isZoomAction() const {
        return _zoomAction;
    }

    bool MapInteractionInfo::isRotateAction() const {
        return _rotateAction;
    }

    bool MapInteractionInfo::isTiltAction() const {
        return _tiltAction;
    }
    
    bool MapInteractionInfo::isAnimationStarted() const {
        return _animationStarted;
    }

}
