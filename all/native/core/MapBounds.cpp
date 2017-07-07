#include "MapBounds.h"
#include "core/MapVec.h"
#include "utils/Const.h"

#include <iomanip>
#include <limits>
#include <sstream>
#include <functional>

namespace carto {

    MapBounds::MapBounds() :
        _min(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()),
        _max(-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity())
    {
    }
    
    MapBounds::MapBounds(const MapPos& min, const MapPos& max) :
        _min(min),
        _max(max)
    {
    }
    
    bool MapBounds::operator ==(const MapBounds& mapBounds) const {
        return _min == mapBounds._min && _max == mapBounds._max;
    }
    
    bool MapBounds::operator !=(const MapBounds& mapBounds) const {
        return !(*this == mapBounds);
    }
        
    void MapBounds::setBounds(const MapPos& min, const MapPos& max) {
        if (min.getX() > max.getX()) {
            _min.setX(max.getX());
            _max.setX(min.getX());
        } else {
            _min.setX(min.getX());
            _max.setX(max.getX());
        }
        
        if (min.getY() > max.getY()) {
            _min.setY(max.getY());
            _max.setY(min.getY());
        } else {
            _min.setY(min.getY());
            _max.setY(max.getY());
        }
        
        if (min.getZ() > max.getZ()) {
            _min.setZ(max.getZ());
            _max.setZ(min.getZ());
        } else {
            _min.setZ(min.getZ());
            _max.setZ(max.getZ());
        }
    }
        
    MapPos MapBounds::getCenter() const {
        MapVec delta(getDelta());
        delta /= 2;
        return _min + delta;
    }
    
    MapVec MapBounds::getDelta() const {
        return _max - _min;
    }
    
    const MapPos& MapBounds::getMin() const {
        return _min;
    }
    
    void MapBounds::setMin(const MapPos& min) {
        _min = min;
    }
    
    const MapPos& MapBounds::getMax() const {
        return _max;
    }
    
    void MapBounds::setMax(const MapPos& max) {
        _max = max;
    }
    
    bool MapBounds::contains(const MapPos& pos) const {
        if (pos.getX() >= _min.getX() && pos.getX() <= _max.getX() &&
            pos.getY() >= _min.getY() && pos.getY() <= _max.getY() &&
            pos.getZ() >= _min.getZ() && pos.getZ() <= _max.getZ())
        {
            return true;
        }
        return false;
    }
        
    bool MapBounds::contains(const MapBounds& bounds) const {
        if (bounds.getMin().getX() >= _min.getX() && bounds.getMax().getX() <= _max.getX() &&
            bounds.getMin().getY() >= _min.getY() && bounds.getMax().getY() <= _max.getY() &&
            bounds.getMin().getZ() >= _min.getZ() && bounds.getMax().getZ() <= _max.getZ())
        {
            return true;
        }
        return false;
    }
        
    bool MapBounds::intersects(const MapBounds& bounds) const {
        if (bounds.getMax().getX() < _min.getX() || bounds.getMin().getX() > _max.getX() ||
            bounds.getMax().getY() < _min.getY() || bounds.getMin().getY() > _max.getY() ||
            bounds.getMax().getZ() < _min.getZ() || bounds.getMin().getZ() > _max.getZ()) {
            return false;
        }
        return true;
    }
        
    void MapBounds::expandToContain(const MapPos& pos) {
        if (pos.getX() < _min.getX()) {
            _min.setX(pos.getX());
        }
        if (pos.getX() > _max.getX()) {
            _max.setX(pos.getX());
        }
        
        if (pos.getY() < _min.getY()) {
            _min.setY(pos.getY());
        }
        if (pos.getY() > _max.getY()) {
            _max.setY(pos.getY());
        }
        
        if (pos.getZ() < _min.getZ()) {
            _min.setZ(pos.getZ());
        }
        if (pos.getZ() > _max.getZ()) {
            _max.setZ(pos.getZ());
        }
    }
    
    void MapBounds::expandToContain(const MapBounds& bounds) {
        if (bounds.getMin().getX() < _min.getX()) {
            _min.setX(bounds.getMin().getX());
        }
        if (bounds.getMax().getX() > _max.getX()) {
            _max.setX(bounds.getMax().getX());
        }
    
        if (bounds.getMin().getY() < _min.getY()) {
            _min.setY(bounds.getMin().getY());
        }
        if (bounds.getMax().getY() > _max.getY()) {
            _max.setY(bounds.getMax().getY());
        }
    
        if (bounds.getMin().getZ() < _min.getZ()) {
            _min.setZ(bounds.getMin().getZ());
        }
        if (bounds.getMax().getZ() > _max.getZ()) {
            _max.setZ(bounds.getMax().getZ());
        }
    }
    
    int MapBounds::hash() const {
        return static_cast<int>(_min.hash() << 16 ^ _max.hash());
    }
        
    std::string MapBounds::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "MapBounds [minX=" << _min.getX() << ", minY=" << _min.getY() << ", minZ=" << _min.getZ() <<
                ", maxX=" << _max.getX() << ", maxY=" << _max.getY() << ", maxZ=" << _max.getZ() << "]";
        return ss.str();
    }
    
}
