#include "MapPos.h"
#include "core/MapVec.h"
#include "components/Exceptions.h"
#include "utils/GeneralUtils.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <functional>

namespace carto {

    MapPos::MapPos() : _x(0), _y(0), _z(0)
    {
    }
        
    MapPos::MapPos(double x, double y) : _x(x), _y(y), _z(0)
    {
    }
    
    MapPos::MapPos(double x, double y, double z) : _x(x), _y(y), _z(z)
    {
    }
    
    double MapPos::getX() const {
        return _x;
    }

    void MapPos::setX(double x) {
        _x = x;
    }

    double MapPos::getY() const {
        return _y;
    }

    void MapPos::setY(double y) {
        _y = y;
    }

    double MapPos::getZ() const {
        return _z;
    }

    void MapPos::setZ(double z) {
        _z = z;
    }

    double MapPos::operator[](std::size_t n) const {
        switch (n) {
        case 0:
            return _x;
        case 1:
            return _y;
        case 2:
            return _z;
        }
        throw OutOfRangeException("MapPos::operator[]");
    }

    double& MapPos::operator[](std::size_t n) {
        switch (n) {
        case 0:
            return _x;
        case 1:
            return _y;
        case 2:
            return _z;
        }
        throw OutOfRangeException("MapPos::operator[]");
    }

    void MapPos::setCoords(double x, double y) {
        _x = x;
        _y = y;
    }

    void MapPos::setCoords(double x, double y, double z) {
        _x = x;
        _y = y;
        _z = z;
    }

    MapPos& MapPos::operator+=(const MapVec& v) {
        _x += v.getX();
        _y += v.getY();
        _z += v.getZ();
        return *this;
    }
    
    MapPos& MapPos::operator-=(const MapVec& v) {
        _x -= v.getX();
        _y -= v.getY();
        _z -= v.getZ();
        return *this;
    }
    
    MapPos MapPos::operator+(const MapVec& v) const {
        return MapPos(_x + v.getX(), _y + v.getY(), _z + v.getZ());
    }
    
    MapPos MapPos::operator-(const MapVec& v) const {
        return MapPos(_x - v.getX(), _y - v.getY(), _z - v.getZ());
    }
    
    MapVec MapPos::operator-(const MapPos& p) const {
        return MapVec(_x - p.getX(), _y - p.getY(), _z - p.getZ());
    }
    
    bool MapPos::operator==(const MapPos& p) const {
        return _x == p._x && _y == p._y && _z == p._z;
    }

    bool MapPos::operator!=(const MapPos& p) const {
        return !(operator==(p));
    }
    
    int MapPos::hash() const {
        std::hash<double> hasher;
        return static_cast<int>((hasher(_z) << 16) ^ (hasher(_y) << 8) ^ hasher(_x));
    }
    
    std::string MapPos::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "MapPos [x="<< _x << ", y=" << _y << ", z=" << _z << "]";
        return ss.str();
    }
    
}
