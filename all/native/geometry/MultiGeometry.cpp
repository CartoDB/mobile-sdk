#include "MultiGeometry.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <algorithm>

namespace carto {

    MultiGeometry::MultiGeometry(const std::vector<std::shared_ptr<Geometry> >& geometries) :
        Geometry(),
        _geometries(geometries)
    {
        std::for_each(geometries.begin(), geometries.end(), [this](const std::shared_ptr<Geometry>& geometry) {
            _bounds.expandToContain(geometry->getBounds());
        });
    }

    MultiGeometry::~MultiGeometry() {
    }

    MapPos MultiGeometry::getCenterPos() const {
        if (_geometries.empty()) {
            return MapPos();
        }

        MapVec centerPos(0, 0);
        std::for_each(_geometries.begin(), _geometries.end(), [&centerPos](const std::shared_ptr<Geometry>& geometry) {
            centerPos += MapVec(geometry->getCenterPos() - MapPos(0, 0));
        });
        return MapPos(0, 0) + centerPos / static_cast<double>(_geometries.size());
    }

    int MultiGeometry::getGeometryCount() const {
        return static_cast<int>(_geometries.size());
    }
    
    std::shared_ptr<Geometry> MultiGeometry::getGeometry(int index) const {
        if (index < 0 || index >= static_cast<int>(_geometries.size())) {
            throw OutOfRangeException("Geometry index out of range");
        }
        return _geometries[index];
    }

}
