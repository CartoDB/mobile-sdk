#include "PointGeometry.h"

namespace carto {

    PointGeometry::PointGeometry(const MapPos& pos) :
        Geometry(),
        _pos(pos)
    {
        _bounds.setBounds(pos, pos);
    }
    
    PointGeometry::~PointGeometry() {
    }
    
    MapPos PointGeometry::getCenterPos() const {
        return _pos;
    }
    
    const MapPos& PointGeometry::getPos() const {
        return _pos;
    }
    
}
