#include "MultiPolygonGeometry.h"

#include <algorithm>
#include <iterator>

namespace carto {
    
    MultiPolygonGeometry::MultiPolygonGeometry(const std::vector<std::shared_ptr<PolygonGeometry> >& geometries) :
        MultiGeometry(std::vector<std::shared_ptr<Geometry> >())
    {
        _geometries.reserve(geometries.size());
        for (const std::shared_ptr<PolygonGeometry>& geometry : geometries) {
            _geometries.push_back(geometry);
            _bounds.expandToContain(geometry->getBounds());
        }
    }
    
    MultiPolygonGeometry::~MultiPolygonGeometry() {
    }
    
    std::shared_ptr<PolygonGeometry> MultiPolygonGeometry::getGeometry(int index) const {
        return std::static_pointer_cast<PolygonGeometry>(MultiGeometry::getGeometry(index));
    }
    
}
