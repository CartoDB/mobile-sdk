#include "LineStyleBuilder.h"
#include "assets/DefaultLinePNG.h"
#include "graphics/Bitmap.h"

namespace carto {

    LineStyleBuilder::LineStyleBuilder() :
        StyleBuilder(),
        _bitmap(GetDefaultBitmap()),
        _clickWidth(-1),
        _lineEndType(LineEndType::LINE_END_TYPE_ROUND),
        _lineJoinType(LineJoinType::LINE_JOIN_TYPE_MITER),
        _stretchFactor(1),
        _width(12)
    {
    }
    
    LineStyleBuilder::~LineStyleBuilder() {
    }
        
    std::shared_ptr<Bitmap> LineStyleBuilder::getBitmap() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _bitmap;
    }
    
    void LineStyleBuilder::setBitmap(const std::shared_ptr<Bitmap>& bitmap) {
        std::lock_guard<std::mutex> lock(_mutex);
        _bitmap = bitmap;
    }
        
    float LineStyleBuilder::getClickWidth() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _clickWidth;
    }
    
    void LineStyleBuilder::setClickWidth(float clickWidth) {
        std::lock_guard<std::mutex> lock(_mutex);
        _clickWidth = clickWidth;
    }
        
    LineEndType::LineEndType LineStyleBuilder::getLineEndType() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _lineEndType;
    }
    
    void LineStyleBuilder::setLineEndType(LineEndType::LineEndType lineEndType) {
        std::lock_guard<std::mutex> lock(_mutex);
        _lineEndType = lineEndType;
    }
        
    LineJoinType::LineJoinType LineStyleBuilder::getLineJoinType() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _lineJoinType;
    }
    
    void LineStyleBuilder::setLineJoinType(LineJoinType::LineJoinType lineJoinType) {
        std::lock_guard<std::mutex> lock(_mutex);
        _lineJoinType = lineJoinType;
    }
        
    float LineStyleBuilder::getStretchFactor() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _stretchFactor;
    }
    
    void LineStyleBuilder::setStretchFactor(float stretch) {
        std::lock_guard<std::mutex> lock(_mutex);
        _stretchFactor = stretch;
    }
        
    float LineStyleBuilder::getWidth() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _width;
    }
    
    void LineStyleBuilder::setWidth(float width) {
        std::lock_guard<std::mutex> lock(_mutex);
        _width = width;
    }
    
    std::shared_ptr<LineStyle> LineStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::make_shared<LineStyle>(_color, _bitmap, _clickWidth, _lineEndType, _lineJoinType,
                _stretchFactor, _width);
    }
    
    std::shared_ptr<Bitmap> LineStyleBuilder::GetDefaultBitmap() {
        std::lock_guard<std::mutex> lock(_DefaultBitmapMutex);
        if (!_DefaultBitmap) {
            _DefaultBitmap = Bitmap::CreateFromCompressed(default_line_png, default_line_png_len);
        }
        return _DefaultBitmap;
    }
    
    std::shared_ptr<Bitmap> LineStyleBuilder::_DefaultBitmap;
    std::mutex LineStyleBuilder::_DefaultBitmapMutex;
    
}
