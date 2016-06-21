#include "Polygon3DStyleBuilder.h"
#include "styles/Polygon3DStyle.h"

namespace carto {

    Polygon3DStyleBuilder::Polygon3DStyleBuilder() :
        StyleBuilder(),
        _sideColor(),
        _sideColorDefined(false)
    {
    }
    
    Polygon3DStyleBuilder::~Polygon3DStyleBuilder() {
    }
    
    Color Polygon3DStyleBuilder::getSideColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _sideColorDefined ? _sideColor : _color;
    }

    void Polygon3DStyleBuilder::setSideColor(const Color& sideColor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _sideColor = sideColor;
        _sideColorDefined = true;
    }

    std::shared_ptr<Polygon3DStyle> Polygon3DStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::make_shared<Polygon3DStyle>(_color, _sideColorDefined ? _sideColor : _color);
    }
    
}
