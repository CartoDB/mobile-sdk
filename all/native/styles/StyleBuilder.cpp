#include "StyleBuilder.h"

namespace carto {
    
    StyleBuilder::~StyleBuilder() {
    }
    
    Color StyleBuilder::getColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _color;
    }
    
    void StyleBuilder::setColor(const Color& color) {
        std::lock_guard<std::mutex> lock(_mutex);
        _color = color;
    }
        
    StyleBuilder::StyleBuilder() :
        _color(0xFFFFFFFF),
        _mutex()
    {
    }
    
}
