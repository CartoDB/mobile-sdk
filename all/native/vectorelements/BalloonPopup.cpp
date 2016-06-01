#include "BalloonPopup.h"
#include "core/MapPos.h"
#include "core/ScreenPos.h"
#include "core/ScreenBounds.h"
#include "graphics/Color.h"
#include "graphics/Bitmap.h"
#include "graphics/BitmapCanvas.h"
#include "styles/BalloonPopupStyle.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cmath>
#include <algorithm>

namespace carto {
    
    BalloonPopup::BalloonPopup(const std::shared_ptr<Billboard>& baseBillboard, const std::shared_ptr<BalloonPopupStyle>& style,
                               const std::string& title, const std::string& desc) :
        Popup(baseBillboard, style),
        _style(style),
        _title(title),
        _desc(desc)
    {
    }

    BalloonPopup::BalloonPopup(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<BalloonPopupStyle>& style,
                               const std::string& title, const std::string& desc) :
        Popup(geometry, style),
        _style(style),
        _title(title),
        _desc(desc)
    {
    }
        
    BalloonPopup::BalloonPopup(const MapPos& pos, const std::shared_ptr<BalloonPopupStyle>& style,
                               const std::string& title, const std::string& desc) :
        Popup(pos, style),
        _style(style),
        _title(title),
        _desc(desc)
    {
    }

    BalloonPopup::~BalloonPopup() {
    }
        
    std::shared_ptr<Bitmap> BalloonPopup::drawBitmap(const ScreenPos& anchorScreenPos,
                                                     float screenWidth, float screenHeight, float dpToPX) {
        std::unique_lock<std::mutex> lock(_mutex);
        
        float pxToDP = 1 / dpToPX;
        if (_style->isScaleWithDPI()) {
            dpToPX = 1;
        } else {
            pxToDP = 1;
        }
        
        ScreenPos screenPos(anchorScreenPos.getX() * pxToDP, anchorScreenPos.getY() * pxToDP);
        screenWidth *= pxToDP;
        screenHeight *= pxToDP;
        
        int titleFontSize = _style->getTitleFontSize() * dpToPX;
        int descFontSize = _style->getDescriptionFontSize() * dpToPX;
        BalloonPopupMargins titleMargins(_style->getTitleMargins().getLeft() * dpToPX, _style->getTitleMargins().getTop() * dpToPX,
                                         _style->getTitleMargins().getRight() * dpToPX, _style->getTitleMargins().getBottom() * dpToPX);
        BalloonPopupMargins descMargins(_style->getDescriptionMargins().getLeft() * dpToPX, _style->getDescriptionMargins().getTop() * dpToPX,
                                         _style->getDescriptionMargins().getRight() * dpToPX, _style->getDescriptionMargins().getBottom() * dpToPX);
        
        const std::shared_ptr<Bitmap>& leftImage = _style->getLeftImage();;
        int leftImageWidth = 0, leftImageHeight = 0;
        if (leftImage) {
            leftImageWidth = leftImage->getWidth();
            leftImageHeight = leftImage->getHeight();
        }
        
        BalloonPopupMargins leftMargins(_style->getLeftMargins().getLeft() * dpToPX, _style->getLeftMargins().getTop() * dpToPX,
                                        _style->getLeftMargins().getRight() * dpToPX, _style->getLeftMargins().getBottom() * dpToPX);
        
        const std::shared_ptr<Bitmap>& rightImage = _style->getRightImage();
        int rightImageWidth = 0, rightImageHeight = 0;
        if (rightImage) {
            rightImageWidth = rightImage->getWidth();
            rightImageHeight = rightImage->getHeight();
        }
        BalloonPopupMargins rightMargins(_style->getRightMargins().getLeft() * dpToPX, _style->getRightMargins().getTop() * dpToPX,
                                         _style->getRightMargins().getRight() * dpToPX, _style->getRightMargins().getBottom() * dpToPX);
        
        int triangleWidth = _style->getTriangleWidth() * dpToPX;
        int triangleHeight = _style->getTriangleHeight() * dpToPX;
        
        int strokeWidth = _style->getStrokeWidth() * dpToPX;
        
        int screenPadding = SCREEN_PADDING * dpToPX;
        
        // Get colors
        const Color& backgroundColor = _style->getBackgroundColor();
        const Color& leftColor = _style->getLeftColor();
        const Color& rightColor = _style->getRightColor();
        const Color& strokeColor = _style->getStrokeColor();
        
        // Calculate the maximum popup size, adjust with dpi
        int maxPopupWidth = std::min(screenWidth, screenHeight);
        
        // Calcualate maximum title and description width
        int leftMarginWidth = leftMargins.getLeft() + leftMargins.getRight() + leftImageWidth;
        int leftMarginHeight = leftMargins.getTop() + leftMargins.getBottom() + leftImageHeight;
        
        int rightMarginWidth = rightMargins.getLeft() + rightMargins.getRight() + rightImageWidth;
        int rightMarginHeight = rightMargins.getTop() + rightMargins.getBottom() + rightImageHeight;
        
        int titleMarginWidth = 0;
        int titleMarginHeight = 0;
        if (_title != "") {
            titleMarginWidth = titleMargins.getLeft() + titleMargins.getRight();
            titleMarginHeight = titleMargins.getTop() + titleMargins.getBottom();
        }
        float halfStrokeWidth = strokeWidth * 0.5f;
        int maxTitleWidth = maxPopupWidth - screenPadding * 2 - leftMarginWidth - rightMarginWidth - titleMarginWidth - strokeWidth;
        
        int descMarginWidth = 0;
        int descMarginHeight = 0;
        if (_desc != "") {
            descMarginWidth = descMargins.getLeft() + descMargins.getRight();
            descMarginHeight = descMargins.getTop() + descMargins.getBottom();
        }
        int maxDescWidth = maxPopupWidth - screenPadding * 2 - leftMarginWidth - rightMarginWidth - descMarginWidth - strokeWidth;

        // Measure title and description sizes
        ScreenBounds titleSize(ScreenPos(0, 0), ScreenPos(0, 0));
        if (!_title.empty()) {
            BitmapCanvas measureCanvas(0, 0);
            measureCanvas.setFont(_style->getTitleFontName(), titleFontSize);
            titleSize = measureCanvas.measureTextSize(_title, maxTitleWidth, _style->isTitleWrap());
        }
        
        ScreenBounds descSize(ScreenPos(0, 0), ScreenPos(0, 0));
        if (!_desc.empty()) {
            BitmapCanvas measureCanvas(0, 0);
            measureCanvas.setFont(_style->getDescriptionFontName(), descFontSize);
            descSize = measureCanvas.measureTextSize(_desc, maxDescWidth, _style->isDescriptionWrap());
        }
        
        // Calculate triangle height with stroke
        float halfTriangleWidth = triangleWidth * 0.5f;
    	float halfTriangleAngle = std::atan2(triangleWidth, (triangleHeight * 2));
    	int triangleStrokeOffset = triangleHeight + 2 * std::cos(halfTriangleAngle) *
    	strokeWidth * 0.5f / std::cos(Const::PI / 2 - 2 * halfTriangleAngle) + 0.5f;
        
        // Calculate bitmap size and create canvas
        int popupWidth = std::max(titleSize.getWidth() + titleMarginWidth, descSize.getWidth() + descMarginWidth);
        popupWidth += leftMarginWidth + rightMarginWidth + strokeWidth;
        float halfPopupWidth = popupWidth * 0.5f;
        int popupInnerHeight = std::max((float) (titleSize.getHeight() + titleMarginHeight + descSize.getHeight() + descMarginHeight),
                                   (float) std::max(leftMarginHeight, rightMarginHeight));
        int popupHeight = popupInnerHeight + std::max((float) triangleStrokeOffset, halfStrokeWidth) + halfStrokeWidth;
        
        BitmapCanvas canvas(popupWidth, popupHeight);
        
        // Prepare background path
        ScreenBounds backgroundRect(ScreenPos(halfStrokeWidth, halfStrokeWidth),
                                    ScreenPos(popupWidth - halfStrokeWidth, popupHeight - triangleStrokeOffset));
        
        // Calculate anchor point and triangle position
        int triangleOffsetX = 0;
        if (screenPos.getX() + halfPopupWidth + screenPadding > screenWidth) {
            triangleOffsetX = halfPopupWidth - (screenWidth - screenPos.getX()) + screenPadding;
        } else if (screenPos.getX() - halfPopupWidth - screenPadding < 0) {
            triangleOffsetX = screenPos.getX() - halfPopupWidth - screenPadding;
        }
        
        int maxHalfOffsetX = halfPopupWidth - halfTriangleWidth - _style->getCornerRadius() - halfStrokeWidth;
        triangleOffsetX = std::min(maxHalfOffsetX, std::max(-maxHalfOffsetX, triangleOffsetX));
        
        // Prepare triangle path
        float triangleOriginX = triangleOffsetX + halfPopupWidth - halfTriangleWidth;
        float triangleOriginY = popupHeight - triangleStrokeOffset;
        ScreenPos triangleP0(triangleOriginX, triangleOriginY);
        ScreenPos triangleP1(triangleOriginX + triangleWidth, triangleOriginY);
        ScreenPos triangleP2(triangleOriginX + halfTriangleWidth, triangleOriginY + triangleHeight);
        std::vector<ScreenPos> trianglePoints { triangleP0, triangleP1, triangleP2 };
        
        // Stroke background and triangle
        canvas.setDrawMode(BitmapCanvas::STROKE);
        canvas.setColor(strokeColor);
        canvas.setStrokeWidth(strokeWidth);
        canvas.drawRoundRect(backgroundRect, _style->getCornerRadius());
        canvas.drawPolygon(trianglePoints);
        
        // Fill background/2 and triangle
        canvas.setDrawMode(BitmapCanvas::FILL);
        canvas.setColor(backgroundColor);
        canvas.drawRoundRect(backgroundRect, _style->getCornerRadius());
        canvas.drawPolygon(trianglePoints);
        
        if (leftMarginWidth > 0 && leftColor != backgroundColor) {
            // Fill the left area
            ScreenBounds leftRect(ScreenPos(0, 0),
                                  ScreenPos(leftMarginWidth + halfStrokeWidth, popupHeight));
            canvas.pushClipRect(leftRect);
            canvas.setColor(leftColor);
            canvas.drawRoundRect(backgroundRect, _style->getCornerRadius());
            canvas.drawPolygon(trianglePoints);
            canvas.popClipRect();
        }
        
        // Fill right area
        if (rightMarginWidth > 0 && rightColor != backgroundColor) {
            // Fill the right area
            ScreenBounds rightRect(ScreenPos(popupWidth - halfStrokeWidth - rightMarginWidth, 0),
                                   ScreenPos(popupWidth, popupHeight));
            canvas.pushClipRect(rightRect);
            canvas.setColor(rightColor);
            canvas.drawRoundRect(backgroundRect, _style->getCornerRadius());
            canvas.drawPolygon(trianglePoints);
            canvas.popClipRect();
        }
        
        // Draw left image
        if (leftImage) {
            ScreenPos leftOrigin(halfStrokeWidth + leftMargins.getLeft(),
                                 halfStrokeWidth + std::max((float) leftMargins.getTop(), (float) (popupInnerHeight * 0.5f - leftImageHeight * 0.5f)));
            ScreenBounds leftRect(leftOrigin, ScreenPos(leftOrigin.getX() + leftImageWidth, leftOrigin.getY() + leftImageHeight));
            canvas.drawBitmap(leftRect, leftImage);
        }
        
        // Draw right image
        if (rightImage) {
            ScreenPos rightOrigin(popupWidth - halfStrokeWidth - rightMarginWidth + rightMargins.getLeft(),
                                  halfStrokeWidth + std::max((float) rightMargins.getTop(), (float) (popupInnerHeight * 0.5f - rightImageHeight * 0.5f)));
            ScreenBounds rightRect(rightOrigin, ScreenPos(rightOrigin.getX() + rightImageWidth, rightOrigin.getY() + rightImageHeight));
            canvas.drawBitmap(rightRect, rightImage);
        }
        
        // Draw title
        if (!_title.empty()) {
            ScreenPos titlePos(halfStrokeWidth + leftMarginWidth + titleMargins.getLeft(),
                               halfStrokeWidth + titleMargins.getTop());
            canvas.setColor(_style->getTitleColor());
            canvas.setFont(_style->getTitleFontName(), titleFontSize);
            canvas.drawText(_title, titlePos, titleSize.getWidth(), _style->isTitleWrap());
        }
        
        // Draw description
        if (!_desc.empty()) {
            ScreenPos descPos(halfStrokeWidth + leftMarginWidth + descMargins.getLeft(),
                              halfStrokeWidth + titleSize.getHeight() + titleMarginHeight + descMargins.getTop());
            canvas.setColor(_style->getDescriptionColor());
            canvas.setFont(_style->getDescriptionFontName(), descFontSize);
            canvas.drawText(_desc, descPos, descSize.getWidth(), _style->isDescriptionWrap());
        }

        // Done with internal state, update anchor point and build bitmap
        lock.unlock();
        setAnchorPoint(triangleOffsetX / halfPopupWidth, -1);

        return canvas.buildBitmap();
    }
       
    std::string BalloonPopup::getTitle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _title;
    }
        
    void BalloonPopup::setTitle(const std::string& title) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _title = title;
        }
        notifyElementChanged();
    }
        
    std::string BalloonPopup::getDescription() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _desc;
    }

    void BalloonPopup::setDescription(const std::string& desc) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _desc = desc;
        }
        notifyElementChanged();
    }
        
    std::shared_ptr<BalloonPopupStyle> BalloonPopup::getStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _style;
    }

    void BalloonPopup::setStyle(const std::shared_ptr<BalloonPopupStyle>& style) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _style = style;
        }
        Popup::setStyle(style);
    }

}
