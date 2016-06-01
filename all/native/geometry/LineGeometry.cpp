#include "LineGeometry.h"
#include "core/MapPos.h"
#include "utils/GeomUtils.h"
#include "utils/Log.h"

namespace carto {

    LineGeometry::LineGeometry(const std::vector<MapPos>& poses) :
        Geometry(),
        _poses(poses)
    {
        if (_poses.size() < 2) {
            Log::Error("LineGeometry::LineGeometry: Line requires at least 2 vertices");
        }
    
        for (const MapPos& pos : _poses) {
            _bounds.expandToContain(pos);
        }
    }
    
    LineGeometry::~LineGeometry() {
    }
        
    MapPos LineGeometry::getCenterPos() const {
        return GeomUtils::CalculatePointOnLine(_poses);
    }
    
    const std::vector<MapPos>& LineGeometry::getPoses() const {
        return _poses;
    }
    
}
