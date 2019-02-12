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

    int RayIntersectedElement::getOrder() const {
        return _order;
    }

    bool RayIntersectedElement::is3D() const {
        return _is3D;
    }

    double RayIntersectedElement::getDistance(const cglib::vec3<double>& origin) const {
        return cglib::length(origin - _hitPos);
    }
    
}
