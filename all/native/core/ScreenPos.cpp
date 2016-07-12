#include "ScreenPos.h"
#include "components/Exceptions.h"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <functional>

namespace carto {
    
    ScreenPos::ScreenPos() :
        _x(0), _y(0)
    {
    }
    
    ScreenPos::ScreenPos(float x, float y) :
        _x(x), _y(y)
    {
    }

    float ScreenPos::getX() const {
        return _x;
    }
    
    void ScreenPos::setX(float x) {
        _x = x;
    }
    
    float ScreenPos::getY() const {
        return _y;
    }
    
    void ScreenPos::setY(float y) {
        _y = y;
    }
    
    float ScreenPos::operator[](std::size_t n) const {
        switch (n) {
            case 0:
                return _x;
            case 1:
                return _y;
        }
        throw OutOfRangeException("ScreenPos::operator[]");
    }
    
    float& ScreenPos::operator[](std::size_t n) {
        switch (n) {
            case 0:
                return _x;
            case 1:
                return _y;
        }
        throw OutOfRangeException("ScreenPos::operator[]");
    }
    
    void ScreenPos::setCoords(float x, float y) {
        _x = x;
        _y = y;
    }

    bool ScreenPos::operator==(const ScreenPos& p) const {
        return _x == p._x && _y == p._y;
    }
    
    bool ScreenPos::operator!=(const ScreenPos& p) const {
        return !(operator==(p));
    }
    
    int ScreenPos::hash() const {
        std::hash<float> hasher;
        return static_cast<int>((hasher(_x) << 16) ^ hasher(_y));
    }

    std::string ScreenPos::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "ScreenPos [x="<< _x << ", y=" << _y << "]";
        return ss.str();
    }
}

