#include "MultiPolygonGeometry.h"

#include <algorithm>
#include <iterator>

namespace carto {
    
    MultiPolygonGeometry::MultiPolygonGeometry(const std::vector<std::shared_ptr<PolygonGeometry> >& geometries) :
        MultiGeometry(geometries.begin(), geometries.end())
    {
    }
    
    MultiPolygonGeometry::~MultiPolygonGeometry() {
    }
    
    std::shared_ptr<PolygonGeometry> MultiPolygonGeometry::getGeometry(int index) const {
        return std::static_pointer_cast<PolygonGeometry>(MultiGeometry::getGeometry(index));
    }
    
}
