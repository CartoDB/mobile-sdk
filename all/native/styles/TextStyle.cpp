#include "TextStyle.h"

namespace carto {
    
    TextMargins::TextMargins(int left, int top, int right, int bottom) :
        _left(left),
        _top(top),
        _right(right),
        _bottom(bottom)
    {
    }

    TextMargins::~TextMargins() {
    }

    int TextMargins::getLeft() const {
        return _left;
    }

    int TextMargins::getTop() const {
        return _top;
    }

    int TextMargins::getRight() const {
        return _right;
    }

    int TextMargins::getBottom() const {
        return _bottom;
    }
        
    TextStyle::TextStyle(const Color& color,
                         float attachAnchorPointX,
                         float attachAnchorPointY,
                         bool causesOverlap,
                         bool hideIfOverlapped,
                         float horizontalOffset,
                         float verticalOffset,
                         int placementPriority,
                         bool scaleWithDPI,
                         const std::shared_ptr<AnimationStyle>& animStyle,
                         float anchorPointX,
                         float anchorPointY,
                         bool flippable,
                         BillboardOrientation::BillboardOrientation orientationMode,
                         BillboardScaling::BillboardScaling scalingMode,
                         float renderScale,
                         const std::string& fontName,
                         const std::string& textField,
                         float fontSize,
                         bool breakLines,
                         const TextMargins& textMargins,
                         const Color& strokeColor,
                         float strokeWidth,
                         const Color& borderColor,
                         float borderWidth,
                         const Color& backgroundColor) :
        LabelStyle(Color(0xFFFFFFFF),
                   attachAnchorPointX,
                   attachAnchorPointY,
                   causesOverlap,
                   hideIfOverlapped,
                   horizontalOffset,
                   verticalOffset,
                   placementPriority,
                   scaleWithDPI,
                   animStyle,
                   anchorPointX,
                   anchorPointY,
                   flippable,
                   orientationMode,
                   scalingMode,
                   renderScale),
        _fontColor(color),
        _fontName(fontName),
        _textField(textField),
        _fontSize(fontSize),
        _breakLines(breakLines),
        _textMargins(textMargins),
        _strokeColor(strokeColor),
        _strokeWidth(strokeWidth),
        _borderColor(borderColor),
        _borderWidth(borderWidth),
        _backgroundColor(backgroundColor)
    {
    }

    TextStyle::~TextStyle() {
    }
        
    const Color& TextStyle::getFontColor() {
        return _fontColor;
    }

    const std::string& TextStyle::getFontName() const {
        return _fontName;
    }
    
    const std::string& TextStyle::getTextField() const {
        return _textField;
    }

    float TextStyle::getFontSize() const {
        return _fontSize;
    }

    bool TextStyle::isBreakLines() const {
        return _breakLines;
    }

    const TextMargins& TextStyle::getTextMargins() const {
        return _textMargins;
    }

    const Color& TextStyle::getStrokeColor() const {
        return _strokeColor;
    }

    float TextStyle::getStrokeWidth() const {
        return _strokeWidth;
    }

    const Color& TextStyle::getBorderColor() const {
        return _borderColor;
    }

    float TextStyle::getBorderWidth() const {
        return _borderWidth;
    }

    const Color& TextStyle::getBackgroundColor() const {
        return _backgroundColor;
    }

}
