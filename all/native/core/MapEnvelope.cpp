#include "MapEnvelope.h"
#include "core/MapPos.h"
#include "utils/GeomUtils.h"
#include "utils/Log.h"

#include <iomanip>
#include <sstream>
#include <functional>

namespace carto {

    MapEnvelope::MapEnvelope() :
        _bounds(),
        _rectangularConvexHull(true),
        _convexHull()
    {
    }
    
    MapEnvelope::MapEnvelope(const MapBounds& bounds) :
        _bounds(bounds),
        _rectangularConvexHull(true),
        _convexHull()
    {
        _convexHull.push_back(MapPos(bounds.getMin().getX(), bounds.getMin().getY()));
        _convexHull.push_back(MapPos(bounds.getMax().getX(), bounds.getMin().getY()));
        _convexHull.push_back(MapPos(bounds.getMax().getX(), bounds.getMax().getY()));
        _convexHull.push_back(MapPos(bounds.getMin().getX(), bounds.getMax().getY()));
    }
    
    MapEnvelope::MapEnvelope(const std::vector<MapPos>& convexHull) :
        _bounds(),
        _rectangularConvexHull(false),
        _convexHull(convexHull)
    {
        for (const MapPos& pos : _convexHull) {
            _bounds.expandToContain(pos);
        }
    }
    
    MapEnvelope::~MapEnvelope() {
    }
        
    bool MapEnvelope::operator==(const MapEnvelope& envelope) const {
        if (_bounds != envelope._bounds) {
            return false;
        }
        if (_rectangularConvexHull || envelope._rectangularConvexHull) {
            if (_convexHull != envelope._convexHull) { // not exact - if vertex ordering is different, may return false negative
                return false;
            }
        }
        return true;
    }
    
    bool MapEnvelope::operator!=(const MapEnvelope& envelope) const {
        return !(*this == envelope);
    }
    
    const MapBounds& MapEnvelope::getBounds() const {
        return _bounds;
    }
    
    const std::vector<MapPos>& MapEnvelope::getConvexHull() const {
        return _convexHull;
    }
    
    bool MapEnvelope::contains(const MapEnvelope& envelope) const {
        if (!_bounds.contains(envelope.getBounds())) {
            return false;
        }
    
        if (_rectangularConvexHull && envelope._rectangularConvexHull) {
            return false;
        }
    
        if (_rectangularConvexHull) {
            for (const MapPos& pos : envelope._convexHull) {
                if (!_bounds.contains(pos)) {
                    return false;
                }
            }
            return true;
        }
        
        if (envelope._rectangularConvexHull) {
            for (const MapPos& pos : _convexHull) {
                if (!envelope.getBounds().contains(pos)) {
                    return false;
                }
            }
            return true;
        }
    
        for (const MapPos& pos : _convexHull) {
            if (!GeomUtils::PointInsidePolygon(envelope._convexHull, pos)) {
                return false;
            }
        }
        return true;
    }
    
    bool MapEnvelope::intersects(const MapEnvelope& envelope) const {
        if (!_bounds.intersects(envelope.getBounds())) {
            return false;
        }
        
        if (_rectangularConvexHull && envelope._rectangularConvexHull) {
            return true;
        }
        
        return GeomUtils::PolygonsIntersect(_convexHull, envelope._convexHull);
    }
    
    
    int MapEnvelope::hash() const {
        return _bounds.hash();
    }

    std::string MapEnvelope::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "Envelope [";
        if (_rectangularConvexHull) {
            ss << _bounds.toString();
        } else {
            std::vector<MapPos>::const_iterator it;
            for (it = _convexHull.begin(); it != _convexHull.end(); ++it) {
                const MapPos& pos = *it;
                ss << (it == _convexHull.begin() ? "" : ", ") << pos.toString();
            }
        }
        ss << "]";
        return ss.str();
    }
    
}
