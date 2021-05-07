#include "RayIntersectedElement.h"
#include "layers/Layer.h"

namespace carto {
    
    RayIntersectedElement::~RayIntersectedElement() {
    }
    
    const std::shared_ptr<Layer>& RayIntersectedElement::getLayer() const {
        return _layer;
    }

    const cglib::vec3<double>& RayIntersectedElement::getHitPos() const {
        return _hitPos;
    }

    const cglib::vec3<double>& RayIntersectedElement::getElementPos() const {
        return _elementPos;
    }

    bool RayIntersectedElement::is3DElement() const {
        return _is3DElement;
    }

    double RayIntersectedElement::getDistance(const cglib::vec3<double>& origin) const {
        return cglib::length(origin - _hitPos);
    }
    
}
