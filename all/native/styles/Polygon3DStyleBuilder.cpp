#include "Polygon3DStyleBuilder.h"
#include "styles/Polygon3DStyle.h"

namespace carto {

    Polygon3DStyleBuilder::Polygon3DStyleBuilder() :
        StyleBuilder()
    {
    }
    
    Polygon3DStyleBuilder::~Polygon3DStyleBuilder() {
    }
    
    std::shared_ptr<Polygon3DStyle> Polygon3DStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::make_shared<Polygon3DStyle>(_color);
    }
    
}
