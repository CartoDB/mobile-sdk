#include "ScreenBounds.h"
#include "utils/Const.h"

#include <iomanip>
#include <limits>
#include <sstream>
#include <functional>

namespace carto {
    
    ScreenBounds::ScreenBounds() :
        _min(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()),
        _max(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity())
    {
    }
    
    ScreenBounds::ScreenBounds(const ScreenPos& min, const ScreenPos& max) :
        _min(min),
        _max(max)
    {
    }
    
    ScreenBounds::~ScreenBounds() {
    }
    
    bool ScreenBounds::operator==(const ScreenBounds& bounds) const {
        return _min == bounds._min && _max == bounds._max;
    }
    
    bool ScreenBounds::operator!=(const ScreenBounds& bounds) const {
        return !operator==(bounds);
    }
    
    void ScreenBounds::setBounds(const ScreenPos& min, const ScreenPos& max) {
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
    }
    
    ScreenPos ScreenBounds::getCenter() const {
        return ScreenPos((_min.getX() + _max.getX()) * 0.5f, (_min.getY() + _max.getY()) * 0.5f);
    }

    float ScreenBounds::getWidth() const {
        return _max.getX() - _min.getX();
    }
    
    float ScreenBounds::getHeight() const {
        return _max.getY() - _min.getY();
    }
    
    const ScreenPos& ScreenBounds::getMin() const {
        return _min;
    }
    
    void ScreenBounds::setMin(const ScreenPos& min) {
        _min = min;
    }
    
    const ScreenPos& ScreenBounds::getMax() const {
        return _max;
    }
    
    void ScreenBounds::setMax(const ScreenPos& max) {
        _max = max;
    }
    
    bool ScreenBounds::contains(const ScreenPos& pos) const {
        if (pos.getX() > _min.getX() && pos.getX() < _max.getX() &&
            pos.getY() > _min.getY() && pos.getY() < _max.getY())
        {
            return true;
        }
        return false;
    }
    
    bool ScreenBounds::contains(const ScreenBounds& bounds) const {
        if (bounds.getMin().getX() >= _min.getX() && bounds.getMax().getX() <= _max.getX() &&
            bounds.getMin().getY() >= _min.getY() && bounds.getMax().getY() <= _max.getY())
        {
            return true;
        }
        return false;
    }
    
    bool ScreenBounds::intersects(const ScreenBounds& bounds) const {
        if (bounds.getMax().getX() < _min.getX() || bounds.getMin().getX() > _max.getX() ||
            bounds.getMax().getY() < _min.getY() || bounds.getMin().getY() > _max.getY())
        {
            return false;
        }
        return true;
    }
    
    void ScreenBounds::expandToContain(const ScreenPos& pos) {
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
    }
    
    void ScreenBounds::expandToContain(const ScreenBounds& bounds) {
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
    }
    
    int ScreenBounds::hash() const {
        return static_cast<int>((_min.hash() << 16) ^ _max.hash());
    }

    std::string ScreenBounds::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "ScreenBounds [minX=" << _min.getX() << ", minY=" << _min.getY() << ", maxX=" << _max.getX() << ", maxY=" << _max.getY() << "]";
        return ss.str();
    }
    
}
