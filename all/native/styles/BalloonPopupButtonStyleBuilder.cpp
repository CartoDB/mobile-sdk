#include "BalloonPopupButtonStyleBuilder.h"

namespace carto {

    BalloonPopupButtonStyleBuilder::BalloonPopupButtonStyleBuilder() :
        _buttonWidth(-1),
        _cornerRadius(3),
        _textColor(Color(0xFF000000)),
        _textFontName("HelveticaNeue-Light"),
        _textFontSize(16),
        _textMargins(BalloonPopupMargins(8, 4, 8, 8)),
        _strokeColor(Color(0xFF000000)),
        _strokeWidth(1)
    {
    }

    BalloonPopupButtonStyleBuilder::~BalloonPopupButtonStyleBuilder() {
    }
        
    int BalloonPopupButtonStyleBuilder::getButtonWidth() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _buttonWidth;
    }

    void BalloonPopupButtonStyleBuilder::setButtonWidth(int buttonWidth) {
        std::lock_guard<std::mutex> lock(_mutex);
        _buttonWidth = buttonWidth;
    }

    int BalloonPopupButtonStyleBuilder::getCornerRadius() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _cornerRadius;
    }

    void BalloonPopupButtonStyleBuilder::setCornerRadius(int cornerRadius) {
        std::lock_guard<std::mutex> lock(_mutex);
        _cornerRadius = cornerRadius;
    }

    Color BalloonPopupButtonStyleBuilder::getTextColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _textColor;
    }

    void BalloonPopupButtonStyleBuilder::setTextColor(const Color& textColor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _textColor = textColor;
    }

    std::string BalloonPopupButtonStyleBuilder::getTextFontName() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _textFontName;
    }

    void BalloonPopupButtonStyleBuilder::setTextFontName(const std::string& textFontName) {
        std::lock_guard<std::mutex> lock(_mutex);
        _textFontName = textFontName;
    }

    int BalloonPopupButtonStyleBuilder::getTextFontSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _textFontSize;
    }

    void BalloonPopupButtonStyleBuilder::setTextFontSize(int textFontSize) {
        std::lock_guard<std::mutex> lock(_mutex);
        _textFontSize = textFontSize;
    }

    BalloonPopupMargins BalloonPopupButtonStyleBuilder::getTextMargins() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _textMargins;
    }

    void BalloonPopupButtonStyleBuilder::setTextMargins(const BalloonPopupMargins& textMargins) {
        std::lock_guard<std::mutex> lock(_mutex);
        _textMargins = textMargins;
    }
        
    Color BalloonPopupButtonStyleBuilder::getStrokeColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _strokeColor;
    }

    void BalloonPopupButtonStyleBuilder::setStrokeColor(const Color& strokeColor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _strokeColor = strokeColor;
    }

    int BalloonPopupButtonStyleBuilder::getStrokeWidth() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _strokeWidth;
    }

    void BalloonPopupButtonStyleBuilder::setStrokeWidth(int strokeWidth) {
        std::lock_guard<std::mutex> lock(_mutex);
        _strokeWidth = strokeWidth;
    }

    std::shared_ptr<BalloonPopupButtonStyle> BalloonPopupButtonStyleBuilder::buildStyle() {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::make_shared<BalloonPopupButtonStyle>(_color,
                                                         _buttonWidth,
                                                         _cornerRadius,
                                                         _textColor,
                                                         _textFontName,
                                                         _textFontSize,
                                                         _textMargins,
                                                         _strokeColor,
                                                         _strokeWidth);
    }

}
