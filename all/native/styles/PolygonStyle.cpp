#include "PolygonStyle.h"

namespace carto {

    PolygonStyle::PolygonStyle(const Color& color, const std::shared_ptr<Bitmap>& bitmap,
            const std::shared_ptr<LineStyle>& lineStyle) :
        Style(color),
        _bitmap(bitmap),
        _lineStyle(lineStyle)
    {
    }
    
    PolygonStyle::~PolygonStyle() {
    }
    
    std::shared_ptr<Bitmap> PolygonStyle::getBitmap() const {
        return _bitmap;
    }
    
    std::shared_ptr<LineStyle> PolygonStyle::getLineStyle() const {
        return _lineStyle;
    }
    
}
