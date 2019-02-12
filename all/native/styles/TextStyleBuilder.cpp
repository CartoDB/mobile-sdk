#include "TextStyleBuilder.h"

namespace carto {

    TextStyleBuilder::TextStyleBuilder() :
        LabelStyleBuilder(),
        _fontName("Helvetica"),
        _textField(),
        _fontSize(20.0f),
        _breakLines(false),
        _textMargins(0, 0, 0, 0),
        _strokeColor(0xFFFFFFFF),
        _strokeWidth(3),
        _borderColor(0xFF000000),
        _borderWidth(0),
        _backgroundColor()
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
    
    void TextStyleBuilder::setTextField(const std::string& field) {
        std::lock_guard<std::mutex> lock(_mutex);
        _textField = field;
    }

    float TextStyleBuilder::getFontSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _fontSize;
    }

    void TextStyleBuilder::setFontSize(float size) {
        std::lock_guard<std::mutex> lock(_mutex);
        _fontSize = size;
    }

    bool TextStyleBuilder::isBreakLines() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _breakLines;
    }

    void TextStyleBuilder::setBreakLines(bool enable) {
        std::lock_guard<std::mutex> lock(_mutex);
        _breakLines = enable;
    }

    TextMargins TextStyleBuilder::getTextMargins() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _textMargins;
    }

    void TextStyleBuilder::setTextMargins(const TextMargins& textMargins) {
        std::lock_guard<std::mutex> lock(_mutex);
        _textMargins = textMargins;
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

    Color TextStyleBuilder::getBorderColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _borderColor;
    }

    void TextStyleBuilder::setBorderColor(const Color& borderColor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _borderColor = borderColor;
    }

    float TextStyleBuilder::getBorderWidth() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _borderWidth;
    }

    void TextStyleBuilder::setBorderWidth(float borderWidth) {
        std::lock_guard<std::mutex> lock(_mutex);
        _borderWidth = borderWidth;
    }

    Color TextStyleBuilder::getBackgroundColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _backgroundColor;
    }

    void TextStyleBuilder::setBackgroundColor(const Color& backgroundColor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _backgroundColor = backgroundColor;
    }

    std::shared_ptr<TextStyle> TextStyleBuilder::buildStyle() {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::make_shared<TextStyle>(_color,
                                           _attachAnchorPointX,
                                           _attachAnchorPointY,
                                           _causesOverlap,
                                           _hideIfOverlapped,
                                           _horizontalOffset,
                                           _verticalOffset,
                                           _placementPriority,
                                           _scaleWithDPI,
                                           _animationStyle,
                                           _anchorPointX,
                                           _anchorPointY,
                                           _flippable,
                                           _orientationMode,
                                           _scalingMode,
                                           _renderScale,
                                           _fontName,
                                           _textField,
                                           _fontSize,
                                           _breakLines,
                                           _textMargins,
                                           _strokeColor,
                                           _strokeWidth,
                                           _borderColor,
                                           _borderWidth,
                                           _backgroundColor);
    }

}
