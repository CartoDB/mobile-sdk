#include "MultiGeometry.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <algorithm>

namespace carto {

    MultiGeometry::MultiGeometry(std::vector<std::shared_ptr<Geometry> > geometries) :
        Geometry(),
        _geometries(std::move(geometries))
    {
        for (const std::shared_ptr<Geometry>& geometry : _geometries) {
            _bounds.expandToContain(geometry->getBounds());
        }
    }

    MultiGeometry::~MultiGeometry() {
    }

    MapPos MultiGeometry::getCenterPos() const {
        if (_geometries.empty()) {
            return MapPos();
        }

        MapVec centerPos(0, 0);
        for (const std::shared_ptr<Geometry>& geometry : _geometries) {
            centerPos += MapVec(geometry->getCenterPos() - MapPos(0, 0));
        }
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
