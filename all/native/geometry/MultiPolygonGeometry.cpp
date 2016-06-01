#include "MultiPolygonGeometry.h"

#include <algorithm>
#include <iterator>

namespace carto {
    
    MultiPolygonGeometry::MultiPolygonGeometry(const std::vector<std::shared_ptr<PolygonGeometry> >& geometries) :
        MultiGeometry(std::vector<std::shared_ptr<Geometry> >())
    {
        _geometries.reserve(geometries.size());
        std::copy(geometries.begin(), geometries.end(), std::back_inserter(_geometries));
    }
    
    MultiPolygonGeometry::~MultiPolygonGeometry() {
    }
    
    std::shared_ptr<PolygonGeometry> MultiPolygonGeometry::getGeometry(int index) const {
        return std::static_pointer_cast<PolygonGeometry>(MultiGeometry::getGeometry(index));
    }
    
}
