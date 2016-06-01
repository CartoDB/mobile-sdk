#include "TextStyleBuilder.h"

namespace carto {

    TextStyleBuilder::TextStyleBuilder() :
        LabelStyleBuilder(),
    	_fontName("Helvetica"),
        _textField(),
    	_fontSize(20),
    	_strokeColor(0xFFFFFFFF),
    	_strokeWidth(3)
    {
        setHideIfOverlapped(true);
        setColor(Color(0xFF000000));
    }

    TextStyleBuilder::~TextStyleBuilder() {
    }

    std::string TextStyleBuilder::getFontName() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _fontName;
    }

    void TextStyleBuilder::setFontName(const std::string& fontName) {
        std::lock_guard<std::mutex> lock(_mutex);
        _fontName = fontName;
    }
    
    std::string TextStyleBuilder::getTextField() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _textField;
    }
    
    void TextStyleBuilder::setTextField(const std::string &field) {
        std::lock_guard<std::mutex> lock(_mutex);
        _textField = field;
    }

    int TextStyleBuilder::getFontSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _fontSize;
    }

    void TextStyleBuilder::setFontSize(int fontSize) {
        std::lock_guard<std::mutex> lock(_mutex);
        _fontSize = fontSize;
    }

    Color TextStyleBuilder::getStrokeColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _strokeColor;
    }

    void TextStyleBuilder::setStrokeColor(const Color& strokeColor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _strokeColor = strokeColor;
    }

    float TextStyleBuilder::getStrokeWidth() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _strokeWidth;
    }

    void TextStyleBuilder::setStrokeWidth(float strokeWidth) {
        std::lock_guard<std::mutex> lock(_mutex);
        _strokeWidth = strokeWidth;
    }

    std::shared_ptr<TextStyle> TextStyleBuilder::buildStyle() {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::shared_ptr<TextStyle>(new TextStyle(_color,
                                                        _attachAnchorPointX,
                                                        _attachAnchorPointY,
                                                        _causesOverlap,
                                                        _hideIfOverlapped,
                                                        _horizontalOffset,
                                                        _verticalOffset,
                                                        _placementPriority,
                                                        _scaleWithDPI,
                                                        _anchorPointX,
                                                        _anchorPointY,
                                                        _flippable,
                                                        _orientationMode,
                                                        _scalingMode,
                                                        _fontName,
                                                        _textField,
                                                        _fontSize,
                                                        _strokeColor,
                                                        _strokeWidth));
    }

}
