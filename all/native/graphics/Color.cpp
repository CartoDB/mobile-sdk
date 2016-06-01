#include "Color.h"

#include <sstream>
#include <functional>

namespace carto {

    Color::Color() :
        _r(0),
        _g(0),
        _b(0),
        _a(255)
    {
    }
    
    Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) :
        _r(r),
        _g(g),
        _b(b),
        _a(a)
    {
    }
    
    Color::Color(int color) :
        _r(((color >> 16) & 0xFF)),
        _g(((color >> 8) & 0xFF)),
        _b((color & 0xFF)),
        _a(((color >> 24) & 0xFF))
    {
    }
    
    bool Color::operator ==(const Color& color) const {
        return _r == color._r && _g == color._g && _b == color._b && _a == color._a;
    }
    
    bool Color::operator !=(const Color& color) const {
        return !(operator ==(color));
    }
    
    unsigned char Color::getR() const {
        return _r;
    }
    
    unsigned char Color::getG() const {
        return _g;
    }
    
    unsigned char Color::getB() const {
        return _b;
    }
    
    unsigned char Color::getA() const {
        return _a;
    }
    
    int Color::getARGB() const {
        return ((static_cast<int>(_a) & 0xFF) << 24)
                | ((static_cast<int>(_r) & 0xFF) << 16)
                | ((static_cast<int>(_g) & 0xFF) << 8)
                | ((static_cast<int>(_b) & 0xFF) << 0);
    }
    
    int Color::hash() const {
        return getARGB();
    }
    
    std::string Color::toString() const {
        std::stringstream ss;
        ss << "Color[r=" << static_cast<int>(_r) << ", g=" << static_cast<int>(_g)
                << ", b=" << static_cast<int>(_b) << ", a=" << static_cast<int>(_a) << "]";
        return ss.str();
    }
    
}
