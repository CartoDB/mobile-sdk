#include "MultiPointGeometry.h"

#include <algorithm>
#include <iterator>

namespace carto {

    MultiPointGeometry::MultiPointGeometry(const std::vector<std::shared_ptr<PointGeometry> >& geometries) :
        MultiGeometry(geometries.begin(), geometries.end())
    {
    }

    MultiPointGeometry::~MultiPointGeometry() {
    }

    std::shared_ptr<PointGeometry> MultiPointGeometry::getGeometry(int index) const {
        return std::static_pointer_cast<PointGeometry>(MultiGeometry::getGeometry(index));
    }

}
