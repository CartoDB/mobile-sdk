#include "PolygonGeometry.h"
#include "core/MapPos.h"
#include "utils/GeomUtils.h"
#include "utils/Log.h"

namespace carto {

    PolygonGeometry::PolygonGeometry(const std::vector<MapPos>& poses) :
        Geometry(),
        _rings()
    {
        _rings.push_back(poses);
        if (poses.size() < 3) {
            Log::Error("PolygonGeometry::PolygonGeometry: Polygon requires at least 3 vertices");
        }
    
        // Calculate bounding box
        for (const MapPos& pos : poses) {
            _bounds.expandToContain(pos);
        }
    }
    
    PolygonGeometry::PolygonGeometry(const std::vector<MapPos>& poses, const std::vector<std::vector<MapPos> >& holes) :
        Geometry(),
        _rings()
    {
        _rings.push_back(poses);
        _rings.insert(_rings.end(), holes.begin(), holes.end());
        if (poses.size() < 3) {
            Log::Error("PolygonGeometry::PolygonGeometry: Polygon requires at least 3 vertices");
        }
    
        for (const std::vector<MapPos>& ring : holes) {
            if (ring.size() < 3) {
                Log::Error("PolygonGeometry::PolygonGeometry: All polygon holes require at least 3 vertices");
            }
        }
    
        // Calculate bounding box
        for (const std::vector<MapPos>& ring : _rings) {
            for (const MapPos& pos : ring) {
                _bounds.expandToContain(pos);
            }
        }
    }
    
    PolygonGeometry::PolygonGeometry(const std::vector<std::vector<MapPos> >& rings) :
        Geometry(),
        _rings(rings)
    {
        for (const std::vector<MapPos>& ring : rings) {
            if (ring.size() < 3) {
                Log::Error("PolygonGeometry::PolygonGeometry: All polygon rings require at least 3 vertices");
            }
        }

        // Calculate bounding box
        for (const std::vector<MapPos>& ring : _rings) {
            for (const MapPos& pos : ring) {
                _bounds.expandToContain(pos);
            }
        }
    }

    PolygonGeometry::~PolygonGeometry() {
    }
        
    MapPos PolygonGeometry::getCenterPos() const {
        return GeomUtils::CalculatePointInsidePolygon(getPoses(), getHoles());
    }
    
    const std::vector<MapPos>& PolygonGeometry::getPoses() const {
        static const std::vector<MapPos> EmptyRing;
        if (_rings.empty()) {
            return EmptyRing;
        }
        return _rings[0];
    }
    
    std::vector<std::vector<MapPos> > PolygonGeometry::getHoles() const {
        if (_rings.empty()) {
            return std::vector<std::vector<MapPos> >();
        }
        return std::vector<std::vector<MapPos> >(_rings.begin() + 1, _rings.end());
    }
    
    const std::vector<std::vector<MapPos> >& PolygonGeometry::getRings() const {
        return _rings;
    }

}
