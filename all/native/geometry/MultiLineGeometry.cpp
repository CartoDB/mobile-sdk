#include "MultiLineGeometry.h"

#include <algorithm>
#include <iterator>

namespace carto {
    
    MultiLineGeometry::MultiLineGeometry(const std::vector<std::shared_ptr<LineGeometry> >& geometries) :
        MultiGeometry(geometries.begin(), geometries.end())
    {
    }
    
    MultiLineGeometry::~MultiLineGeometry() {
    }
    
    std::shared_ptr<LineGeometry> MultiLineGeometry::getGeometry(int index) const {
        return std::static_pointer_cast<LineGeometry>(MultiGeometry::getGeometry(index));
    }
    
}
