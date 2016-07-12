#include "PointStyleBuilder.h"
#include "assets/DefaultPointPNG.h"
#include "components/Exceptions.h"
#include "graphics/Bitmap.h"
#include "styles/PointStyle.h"

namespace carto {

    PointStyleBuilder::PointStyleBuilder() :
        StyleBuilder(),
        _bitmap(GetDefaultBitmap()),
        _clickSize(-1),
        _size(20)
    {
    }
    
    PointStyleBuilder::~PointStyleBuilder() {
    }
        
    std::shared_ptr<Bitmap> PointStyleBuilder::getBitmap() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _bitmap;
    }
    
    void PointStyleBuilder::setBitmap(const std::shared_ptr<Bitmap>& bitmap) {
        if (!bitmap) {
            throw NullArgumentException("Null bitmap");
        }

        std::lock_guard<std::mutex> lock(_mutex);
        _bitmap = bitmap;
    }
        
    float PointStyleBuilder::getClickSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _clickSize;
    }
    
    void PointStyleBuilder::setClickSize(float clickSize) {
        std::lock_guard<std::mutex> lock(_mutex);
        _clickSize = clickSize;
    }
        
    float PointStyleBuilder::getSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _size;
    }
    
    void PointStyleBuilder::setSize(float size) {
        std::lock_guard<std::mutex> lock(_mutex);
        _size = size;
    }
    
    std::shared_ptr<PointStyle> PointStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::make_shared<PointStyle>(_color, _bitmap, _clickSize, _size);
    }
    
    std::shared_ptr<Bitmap> PointStyleBuilder::GetDefaultBitmap() {
        std::lock_guard<std::mutex> lock(_DefaultBitmapMutex);
        if (!_DefaultBitmap) {
            _DefaultBitmap = Bitmap::CreateFromCompressed(default_point_png, default_point_png_len);
        }
        return _DefaultBitmap;
    }
    
    std::shared_ptr<Bitmap> PointStyleBuilder::_DefaultBitmap;
    std::mutex PointStyleBuilder::_DefaultBitmapMutex;
 
}
