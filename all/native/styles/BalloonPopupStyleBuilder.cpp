#include "BalloonPopupStyleBuilder.h"
#include "graphics/Bitmap.h"

namespace carto {

    BalloonPopupStyleBuilder::BalloonPopupStyleBuilder() :
        _cornerRadius(3),
        _leftColor(Color(0xFF00B483)),
        _leftImage(nullptr),
        _leftMargins(BalloonPopupMargins(5, 0, 5, 0)),
        _rightColor(Color(0xFFFFFFFF)),
        _rightImage(nullptr),
        _rightMargins(BalloonPopupMargins(0, 0, 0, 0)),
        _titleColor(Color(0xFF000000)),
        _titleFontName("HelveticaNeue-Light"),
        _titleFontSize(16),
        _titleMargins(BalloonPopupMargins(8, 4, 8, 0)),
        _titleWrap(true),
        _descColor(Color(0xFF000000)),
        _descFontName("HelveticaNeue-Light"),
        _descFontSize(14),
        _descMargins(BalloonPopupMargins(8, 0, 8, 6)),
        _descWrap(true),
        _strokeColor(Color(0xFF000000)),
        _strokeWidth(1),
        _triangleWidth(20),
        _triangleHeight(10)
    {
    }

    BalloonPopupStyleBuilder::~BalloonPopupStyleBuilder() {
    }
        
    int BalloonPopupStyleBuilder::getCornerRadius() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _cornerRadius;
    }

    void BalloonPopupStyleBuilder::setCornerRadius(int cornerRadius) {
    	std::lock_guard<std::mutex> lock(_mutex);
        _cornerRadius = cornerRadius;
    }

    Color BalloonPopupStyleBuilder::getLeftColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _leftColor;
    }

    void BalloonPopupStyleBuilder::setLeftColor(const Color& leftColor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _leftColor = leftColor;
    }

    std::shared_ptr<Bitmap> BalloonPopupStyleBuilder::getLeftImage() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _leftImage;
    }

    void BalloonPopupStyleBuilder::setLeftImage(const std::shared_ptr<Bitmap>& leftImage) {
        std::lock_guard<std::mutex> lock(_mutex);
        _leftImage = leftImage;
    }

    BalloonPopupMargins BalloonPopupStyleBuilder::getLeftMargins() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _leftMargins;
    }

    void BalloonPopupStyleBuilder::setLeftMargins(const BalloonPopupMargins& leftMargins) {
        std::lock_guard<std::mutex> lock(_mutex);
        _leftMargins = leftMargins;
    }

    Color BalloonPopupStyleBuilder::getRightColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _rightColor;
    }

    void BalloonPopupStyleBuilder::setRightColor(const Color& rightColor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _rightColor = rightColor;
    }

    std::shared_ptr<Bitmap> BalloonPopupStyleBuilder::getRightImage() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _rightImage;
    }

    void BalloonPopupStyleBuilder::setRightImage(const std::shared_ptr<Bitmap>& rightImage) {
        std::lock_guard<std::mutex> lock(_mutex);
        _rightImage = rightImage;
    }

    BalloonPopupMargins BalloonPopupStyleBuilder::getRightMargins() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _rightMargins;
    }

    void BalloonPopupStyleBuilder::setRightMargins(const BalloonPopupMargins& rightMargins) {
        std::lock_guard<std::mutex> lock(_mutex);
        _rightMargins = rightMargins;
    }

    Color BalloonPopupStyleBuilder::getTitleColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _titleColor;
    }

    void BalloonPopupStyleBuilder::setTitleColor(const Color& titleColor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _titleColor = titleColor;
    }

    std::string BalloonPopupStyleBuilder::getTitleFontName() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _titleFontName;
    }

    void BalloonPopupStyleBuilder::setTitleFontName(const std::string& titleFontName) {
        std::lock_guard<std::mutex> lock(_mutex);
        _titleFontName = titleFontName;
    }

    int BalloonPopupStyleBuilder::getTitleFontSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _titleFontSize;
    }

    void BalloonPopupStyleBuilder::setTitleFontSize(int titleFontSize) {
        std::lock_guard<std::mutex> lock(_mutex);
        _titleFontSize = titleFontSize;
    }

    BalloonPopupMargins BalloonPopupStyleBuilder::getTitleMargins() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _titleMargins;
    }

    void BalloonPopupStyleBuilder::setTitleMargins(const BalloonPopupMargins& titleMargins) {
        std::lock_guard<std::mutex> lock(_mutex);
        _titleMargins = titleMargins;
    }
        
    bool BalloonPopupStyleBuilder::isTitleWrap() const {
    	std::lock_guard<std::mutex> lock(_mutex);
        return _titleWrap;
    }

    void BalloonPopupStyleBuilder::setTitleWrap(bool titleWrap) {
    	std::lock_guard<std::mutex> lock(_mutex);
        _titleWrap = titleWrap;
    }

    Color BalloonPopupStyleBuilder::getDescriptionColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _descColor;
    }

    void BalloonPopupStyleBuilder::setDescriptionColor(const Color& descColor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _descColor = descColor;
    }

    std::string BalloonPopupStyleBuilder::getDescriptionFontName() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _descFontName;
    }

    void BalloonPopupStyleBuilder::setDescriptionFontName(const std::string& descFontName) {
        std::lock_guard<std::mutex> lock(_mutex);
        _descFontName = descFontName;
    }

    int BalloonPopupStyleBuilder::getDescriptionFontSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _descFontSize;
    }

    void BalloonPopupStyleBuilder::setDescriptionFontSize(int descFontSize) {
        std::lock_guard<std::mutex> lock(_mutex);
        _descFontSize = descFontSize;
    }

    BalloonPopupMargins BalloonPopupStyleBuilder::getDescriptionMargins() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _descMargins;
    }

    void BalloonPopupStyleBuilder::setDescriptionMargins(const BalloonPopupMargins& descMargins) {
        std::lock_guard<std::mutex> lock(_mutex);
        _descMargins = descMargins;
    }

    bool BalloonPopupStyleBuilder::isDescriptionWrap() const {
    	std::lock_guard<std::mutex> lock(_mutex);
        return _descWrap;
    }

    void BalloonPopupStyleBuilder::setDescriptionWrap(bool descWrap) {
    	std::lock_guard<std::mutex> lock(_mutex);
        _descWrap = descWrap;
    }

    Color BalloonPopupStyleBuilder::getStrokeColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _strokeColor;
    }

    void BalloonPopupStyleBuilder::setStrokeColor(const Color& strokeColor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _strokeColor = strokeColor;
    }

    int BalloonPopupStyleBuilder::getStrokeWidth() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _strokeWidth;
    }

    void BalloonPopupStyleBuilder::setStrokeWidth(int strokeWidth) {
        std::lock_guard<std::mutex> lock(_mutex);
        _strokeWidth = strokeWidth;
    }

    int BalloonPopupStyleBuilder::getTriangleWidth() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _triangleWidth;
    }

    void BalloonPopupStyleBuilder::setTriangleWidth(int triangleWidth) {
        std::lock_guard<std::mutex> lock(_mutex);
        _triangleWidth = triangleWidth;
    }

    int BalloonPopupStyleBuilder::getTriangleHeight() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _triangleHeight;
    }

    void BalloonPopupStyleBuilder::setTriangleHeight(int triangleHeight) {
        std::lock_guard<std::mutex> lock(_mutex);
        _triangleHeight = triangleHeight;
    }

    std::shared_ptr<BalloonPopupStyle> BalloonPopupStyleBuilder::buildStyle() {
        std::lock_guard<std::mutex> lock(_mutex);
    	return std::shared_ptr<BalloonPopupStyle>(new BalloonPopupStyle(_color,
                                                                        _attachAnchorPointX,
                                                                        _attachAnchorPointY,
                                                                        _causesOverlap,
                                                                        _hideIfOverlapped,
                                                                        _horizontalOffset,
                                                                        _verticalOffset,
                                                                        _placementPriority,
                                                                        _scaleWithDPI,
                                                                        _cornerRadius,
                                                                        _leftColor,
                                                                        _leftImage,
                                                                        _leftMargins,
                                                                        _rightColor,
                                                                        _rightImage,
                                                                        _rightMargins,
                                                                        _titleColor,
                                                                        _titleFontName,
                                                                        _titleFontSize,
                                                                        _titleMargins,
                                                                        _titleWrap,
                                                                        _descColor,
                                                                        _descFontName,
                                                                        _descFontSize,
                                                                        _descMargins,
                                                                        _descWrap,
                                                                        _strokeColor,
                                                                        _strokeWidth,
                                                                        _triangleWidth,
                                                                        _triangleHeight));
    }

}
