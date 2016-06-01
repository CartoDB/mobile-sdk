#include "RayIntersectedElement.h"
#include "layers/Layer.h"

namespace carto {
    
    RayIntersectedElement::~RayIntersectedElement() {
    }
    
    const std::shared_ptr<Layer>& RayIntersectedElement::getLayer() const {
        return _layer;
    }

    const MapPos& RayIntersectedElement::getHitPos() const {
        return _hitPos;
    }

    const MapPos& RayIntersectedElement::getElementPos() const {
        return _elementPos;
    }

    int RayIntersectedElement::getOrder() const {
        return _order;
    }

    double RayIntersectedElement::getDistance(const MapPos& origin) const {
        return (origin - _hitPos).length();
    }
    
}
