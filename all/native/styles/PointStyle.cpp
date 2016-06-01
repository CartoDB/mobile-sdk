#include "PointStyle.h"

namespace carto {

    PointStyle::PointStyle(const Color& color, const std::shared_ptr<Bitmap>& bitmap, float clickSize, float size) :
        Style(color),
        _bitmap(bitmap),
        _clickSize(clickSize),
        _size(size)
    {
    }
    
    PointStyle::~PointStyle() {
    }
    
    std::shared_ptr<Bitmap> PointStyle::getBitmap() const {
        return _bitmap;
    }
        
    float PointStyle::getClickSize() const {
        return _clickSize;
    }
    
    float PointStyle::getSize() const {
        return _size;
    }
    
}
