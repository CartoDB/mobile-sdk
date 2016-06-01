#include "MapRange.h"

#include <limits>
#include <sstream>

namespace carto {

    MapRange::MapRange() :
        _min(std::numeric_limits<float>::infinity()),
        _max(-std::numeric_limits<float>::infinity())
    {
    }
    
    MapRange::MapRange(float min, float max) :
        _min(min),
        _max(max)
    {
    
        if (_min > _max) {
            _min = max;
            _max = min;
        }
    }
    
    bool MapRange::operator ==(const MapRange& mapRange) const {
        return _min == mapRange._min && _max == mapRange._max;
    }
    
    bool MapRange::operator !=(const MapRange& mapRange) const {
        return !(operator==(mapRange));
    }
    
    float MapRange::getMin() const {
        return _min;
    }
    
    void MapRange::setMin(float min) {
        _min = min;
    }
    
    float MapRange::getMax() const {
        return _max;
    }
    
    void MapRange::setMax(float max) {
        _max = max;
    }
    
    bool MapRange::inRange(float val) const {
        return val >= _min && val < _max;
    }
    
    void MapRange::setRange(float min, float max) {
        if (_min > _max) {
            _min = max;
            _max = min;
        } else {
            _min = min;
            _max = max;
        }
    }
    
    float MapRange::length() const{
        return _max - _min;
    }
    
    int MapRange::hash() const {
        std::hash<float> hasher;
        return static_cast<int>((hasher(_min) << 16) ^ hasher(_max));
    }

    std::string MapRange::toString() const {
        std::stringstream ss;
        ss << "MapRange [min=" << _min << ", max=" << _max << "]";
        return ss.str();
    }
    
}
