#ifdef _CARTO_EDITABLE_SUPPORT

#include "VectorElementDragInfo.h"

namespace carto {
    
    VectorElementDragInfo::VectorElementDragInfo(const std::shared_ptr<VectorElement>& vectorElement, VectorElementDragMode::VectorElementDragMode dragMode, const ScreenPos& screenPos, const MapPos& mapPos) :
        _dragMode(dragMode),
        _screenPos(screenPos),
        _mapPos(mapPos),
        _vectorElement(vectorElement)
    {
    }
    
    VectorElementDragInfo::~VectorElementDragInfo() {
    }
    
    VectorElementDragMode::VectorElementDragMode VectorElementDragInfo::getDragMode() const {
        return _dragMode;
    }
    
    const ScreenPos& VectorElementDragInfo::getScreenPos() const {
        return _screenPos;
    }

    const MapPos& VectorElementDragInfo::getMapPos() const {
        return _mapPos;
    }
    
    std::shared_ptr<VectorElement> VectorElementDragInfo::getVectorElement() const {
        return _vectorElement;
    }
    
}

#endif
