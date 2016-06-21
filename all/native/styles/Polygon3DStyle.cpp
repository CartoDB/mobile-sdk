#include "Polygon3DStyle.h"

namespace carto {

    Polygon3DStyle::Polygon3DStyle(const Color& color, const Color& sideColor) :
        Style(color),
        _sideColor(sideColor)
    {
    }
    
    Polygon3DStyle::~Polygon3DStyle() {
    }

    const Color& Polygon3DStyle::getSideColor() const {
        return _sideColor;
    }
    
}
