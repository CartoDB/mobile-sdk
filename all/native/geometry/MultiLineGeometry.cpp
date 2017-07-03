#include "MultiLineGeometry.h"

#include <algorithm>
#include <iterator>

namespace carto {
    
    MultiLineGeometry::MultiLineGeometry(const std::vector<std::shared_ptr<LineGeometry> >& geometries) :
        MultiGeometry(std::vector<std::shared_ptr<Geometry> >())
    {
        _geometries.reserve(geometries.size());
        for (const std::shared_ptr<LineGeometry>& geometry : geometries) {
            _geometries.push_back(geometry);
            _bounds.expandToContain(geometry->getBounds());
        }
    }
    
    MultiLineGeometry::~MultiLineGeometry() {
    }
    
    std::shared_ptr<LineGeometry> MultiLineGeometry::getGeometry(int index) const {
        return std::static_pointer_cast<LineGeometry>(MultiGeometry::getGeometry(index));
    }
    
}
