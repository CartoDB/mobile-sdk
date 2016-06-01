#include "Style.h"

namespace carto {

    Style::~Style() {
    }
    
    const Color& Style::getColor() const {
        return _color;
    }
    
    Style::Style(const Color& color) :
        _color(color)
    {
    }
    
}
