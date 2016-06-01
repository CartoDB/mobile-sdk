#include "MultiPointGeometry.h"

#include <algorithm>
#include <iterator>

namespace carto {

    MultiPointGeometry::MultiPointGeometry(const std::vector<std::shared_ptr<PointGeometry> >& geometries) :
        MultiGeometry(std::vector<std::shared_ptr<Geometry> >())
    {
        _geometries.reserve(geometries.size());
        std::copy(geometries.begin(), geometries.end(), std::back_inserter(_geometries));
    }

    MultiPointGeometry::~MultiPointGeometry() {
    }

    std::shared_ptr<PointGeometry> MultiPointGeometry::getGeometry(int index) const {
        return std::static_pointer_cast<PointGeometry>(MultiGeometry::getGeometry(index));
    }

}
