#include "TextStyle.h"

namespace carto {
    
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
                         const Color& strokeColor,
                         float strokeWidth) :
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
        _strokeColor(strokeColor),
        _strokeWidth(strokeWidth)
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

    const Color& TextStyle::getStrokeColor() const {
        return _strokeColor;
    }

    float TextStyle::getStrokeWidth() const {
        return _strokeWidth;
    }

}
