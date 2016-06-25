#include "LineStyle.h"

namespace carto {

    LineStyle::LineStyle(const Color& color, const std::shared_ptr<Bitmap>& bitmap, float clickWidth,
            LineEndType::LineEndType lineEndType, LineJoinType::LineJoinType lineJoinType,
            float stretchFactor, float width) :
        Style(color),
        _bitmap(bitmap),
        _clickWidth(clickWidth),
        _lineEndType(lineEndType),
        _lineJoinType(lineJoinType),
        _stretchFactor(stretchFactor),
        _width(width)
    {
    }
    
    LineStyle::~LineStyle() {
    }
    
    std::shared_ptr<Bitmap> LineStyle::getBitmap() const {
        return _bitmap;
    }
        
    float LineStyle::getClickWidth() const {
        return _clickWidth;
    }
    
    LineEndType::LineEndType LineStyle::getLineEndType() const {
        return _lineEndType;
    }
    
    LineJoinType::LineJoinType LineStyle::getLineJoinType() const {
        return _lineJoinType;
    }
    
    float LineStyle::getStretchFactor() const {
        return _stretchFactor;
    }
    
    float LineStyle::getWidth() const {
        return _width;
    }
    
}
