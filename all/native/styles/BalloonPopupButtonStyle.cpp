#include "BalloonPopupButtonStyle.h"

namespace carto {
    
    BalloonPopupButtonStyle::BalloonPopupButtonStyle(const Color& color,
                                                     int cornerRadius,
                                                     const Color& textColor,
                                                     const std::string& textFontName,
                                                     int textFontSize,
                                                     const BalloonPopupMargins& textMargins,
                                                     const Color& strokeColor,
                                                     int strokeWidth) :
        Style(Color(0xFFFFFFFF)),
        _backgroundColor(color),
        _cornerRadius(cornerRadius),
        _textColor(textColor),
        _textFontName(textFontName),
        _textFontSize(textFontSize),
        _textMargins(textMargins),
        _strokeColor(strokeColor),
        _strokeWidth(strokeWidth)
    {
    }

    BalloonPopupButtonStyle::~BalloonPopupButtonStyle() {
    }
        
    const Color& BalloonPopupButtonStyle::getBackgroundColor() const {
        return _backgroundColor;
    }
        
    int BalloonPopupButtonStyle::getCornerRadius() const {
        return _cornerRadius;
    }
        
    const Color& BalloonPopupButtonStyle::getTextColor() const {
        return _textColor;
    }

    const std::string& BalloonPopupButtonStyle::getTextFontName() const {
        return _textFontName;
    }

    int BalloonPopupButtonStyle::getTextFontSize() const {
        return _textFontSize;
    }

    const BalloonPopupMargins& BalloonPopupButtonStyle::getTextMargins() const {
        return _textMargins;
    }
        
    const Color& BalloonPopupButtonStyle::getStrokeColor() const {
        return _strokeColor;
    }

    int BalloonPopupButtonStyle::getStrokeWidth() const {
        return _strokeWidth;
    }

}
