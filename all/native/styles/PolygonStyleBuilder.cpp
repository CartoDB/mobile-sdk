#include "PolygonStyleBuilder.h"
#include "assets/DefaultPolygonPNG.h"
#include "graphics/Bitmap.h"
#include "styles/PolygonStyle.h"

namespace carto {

    PolygonStyleBuilder::PolygonStyleBuilder() :
        StyleBuilder(),
        _bitmap(GetDefaultBitmap()),
        _lineStyle()
    {
    }
    
    PolygonStyleBuilder::~PolygonStyleBuilder() {
    }
    
    /*void PolygonStyleBuilder::setBitmap(const std::shared_ptr<Bitmap>& bitmap) {
        _bitmap = bitmap;
    }*/
        
    std::shared_ptr<LineStyle> PolygonStyleBuilder::getLineStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _lineStyle;
    }
    
    void PolygonStyleBuilder::setLineStyle(const std::shared_ptr<LineStyle>& lineStyle) {
        std::lock_guard<std::mutex> lock(_mutex);
        _lineStyle = lineStyle;
    }
    
    std::shared_ptr<PolygonStyle> PolygonStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::make_shared<PolygonStyle>(_color, _bitmap, _lineStyle);
    }
    
    std::shared_ptr<Bitmap> PolygonStyleBuilder::GetDefaultBitmap() {
        std::lock_guard<std::mutex> lock(_DefaultBitmapMutex);
        if (!_DefaultBitmap) {
            _DefaultBitmap = Bitmap::CreateFromCompressed(default_polygon_png, default_polygon_png_len);
        }
        return _DefaultBitmap;
    }
    
    std::shared_ptr<Bitmap> PolygonStyleBuilder::_DefaultBitmap;
    std::mutex PolygonStyleBuilder::_DefaultBitmapMutex;
    
}
