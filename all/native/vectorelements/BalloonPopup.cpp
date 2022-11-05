#include "BalloonPopup.h"
#include "components/Exceptions.h"
#include "core/MapPos.h"
#include "core/ScreenPos.h"
#include "core/ScreenBounds.h"
#include "graphics/Color.h"
#include "graphics/Bitmap.h"
#include "graphics/BitmapCanvas.h"
#include "styles/BalloonPopupStyle.h"
#include "styles/BalloonPopupButtonStyle.h"
#include "ui/BalloonPopupButtonClickInfo.h"
#include "vectorelements/BalloonPopupButton.h"
#include "vectorelements/BalloonPopupEventListener.h"
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
        _desc(desc),
        _buttons(),
        _buttonRects(),
        _balloonPopupEventListener()
    {
    }

    BalloonPopup::BalloonPopup(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<BalloonPopupStyle>& style,
                               const std::string& title, const std::string& desc) :
        Popup(geometry, style),
        _style(style),
        _title(title),
        _desc(desc),
        _buttons(),
        _buttonRects(),
        _balloonPopupEventListener()
    {
    }
        
    BalloonPopup::BalloonPopup(const MapPos& pos, const std::shared_ptr<BalloonPopupStyle>& style,
                               const std::string& title, const std::string& desc) :
        Popup(pos, style),
        _style(style),
        _title(title),
        _desc(desc),
        _buttons(),
        _buttonRects(),
        _balloonPopupEventListener()
    {
    }

    BalloonPopup::~BalloonPopup() {
    }
        
    std::string BalloonPopup::getTitle() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _title;
    }
        
    void BalloonPopup::setTitle(const std::string& title) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _title = title;
        }
        notifyElementChanged();
    }
        
    std::string BalloonPopup::getDescription() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _desc;
    }

    void BalloonPopup::setDescription(const std::string& desc) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _desc = desc;
        }
        notifyElementChanged();
    }
        
    std::shared_ptr<BalloonPopupStyle> BalloonPopup::getStyle() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _style;
    }

    void BalloonPopup::setStyle(const std::shared_ptr<BalloonPopupStyle>& style) {
        if (!style) {
            throw NullArgumentException("Null style");
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _style = style;
        }
        Popup::setStyle(style);
    }

    void BalloonPopup::clearButtons() {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _buttons.clear();
            _buttonRects.clear();
        }
        notifyElementChanged();
    }

    void BalloonPopup::addButton(const std::shared_ptr<BalloonPopupButton>& button) {
        if (!button) {
            throw NullArgumentException("Null button");
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            if (std::find(_buttons.begin(), _buttons.end(), button) == _buttons.end()) {
                _buttons.push_back(button);
            }
            // Note: _buttonRects will be updated when the button is drawn
        }
        notifyElementChanged();
    }

    void BalloonPopup::replaceButton(const std::shared_ptr<BalloonPopupButton>& oldButton, const std::shared_ptr<BalloonPopupButton>& newButton) {
        if (!oldButton || !newButton) {
            throw NullArgumentException("Null button");
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            auto it = std::find(_buttons.begin(), _buttons.end(), oldButton);
            if (it != _buttons.end()) {
                *it = newButton;
                _buttonRects.erase(oldButton);
            }
        }
        notifyElementChanged();
    }

    void BalloonPopup::removeButton(const std::shared_ptr<BalloonPopupButton>& button) {
        if (!button) {
            throw NullArgumentException("Null button");
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            auto it = std::find(_buttons.begin(), _buttons.end(), button);
            if (it != _buttons.end()) {
                _buttons.erase(it);
                _buttonRects.erase(button);
            }
        }
        notifyElementChanged();
    }

    std::shared_ptr<BalloonPopupEventListener> BalloonPopup::getBalloonPopupEventListener() const {
        return _balloonPopupEventListener.get();
    }

    void BalloonPopup::setBalloonPopupEventListener(const std::shared_ptr<BalloonPopupEventListener>& eventListener) {
        _balloonPopupEventListener.set(eventListener);
    }
        
    bool BalloonPopup::processClick(const ClickInfo& clickInfo, const MapPos& clickPos, const ScreenPos& elementClickPos) {
        DirectorPtr<BalloonPopupEventListener> eventListener = _balloonPopupEventListener;

        if (eventListener) {
            std::vector<std::shared_ptr<BalloonPopupButton> > buttons;
            std::map<std::shared_ptr<BalloonPopupButton>, ScreenBounds> buttonRects;
            {
                std::lock_guard<std::recursive_mutex> lock(_mutex);
                buttons = _buttons;
                buttonRects = _buttonRects;
            }

            // Process the buttons in reverse rendering order
            for (auto it1 = buttons.rbegin(); it1 != buttons.rend(); it1++) {
                const std::shared_ptr<BalloonPopupButton>& button = *it1;

                auto it2 = buttonRects.find(button);
                if (it2 != buttonRects.end()) {
                    if (it2->second.contains(elementClickPos)) {
                        auto balloonPopupButtonClickInfo = std::make_shared<BalloonPopupButtonClickInfo>(clickInfo, button, std::static_pointer_cast<BalloonPopup>(shared_from_this()));
                        if (eventListener->onButtonClicked(balloonPopupButtonClickInfo)) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    std::shared_ptr<Bitmap> BalloonPopup::drawBitmap(const ScreenPos& anchorScreenPos,
                                                     float screenWidth, float screenHeight, float dpToPX) {
        try {
            std::unique_lock<std::recursive_mutex> lock(_mutex);

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
            BalloonPopupMargins buttonMargins(_style->getButtonMargins().getLeft() * dpToPX, _style->getButtonMargins().getTop() * dpToPX,
                                              _style->getButtonMargins().getRight() * dpToPX, _style->getButtonMargins().getBottom() * dpToPX);
        
            const std::shared_ptr<Bitmap>& leftImage = _style->getLeftImage();
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

            // Clear button bounds
            _buttonRects.clear();
        
            // Use actual texts or text fields
            std::string title = _title;
            if (title.empty() && !_style->getTitleField().empty()) {
                Variant value = getMetaDataElement(_style->getTitleField());
                if (value.getType() == VariantType::VARIANT_TYPE_STRING) {
                    title = value.getString();
                } else {
                    title = value.toString();
                }
            }

            std::string desc = _desc;
            if (desc.empty() && !_style->getDescriptionField().empty()) {
                Variant value = getMetaDataElement(_style->getDescriptionField());
                if (value.getType() == VariantType::VARIANT_TYPE_STRING) {
                    desc = value.getString();
                } else {
                    desc = value.toString();
                }
            }

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
            if (!title.empty()) {
                titleMarginWidth = titleMargins.getLeft() + titleMargins.getRight();
                titleMarginHeight = titleMargins.getTop() + titleMargins.getBottom();
            }
            float halfStrokeWidth = strokeWidth * 0.5f;
            int maxTitleWidth = maxPopupWidth - screenPadding * 2 - leftMarginWidth - rightMarginWidth - titleMarginWidth - strokeWidth;
        
            int descMarginWidth = 0;
            int descMarginHeight = 0;
            if (!desc.empty()) {
                descMarginWidth = descMargins.getLeft() + descMargins.getRight();
                descMarginHeight = descMargins.getTop() + descMargins.getBottom();
            }
            int maxDescWidth = maxPopupWidth - screenPadding * 2 - leftMarginWidth - rightMarginWidth - descMarginWidth - strokeWidth;

            // Measure title and description sizes
            ScreenBounds titleSize(ScreenPos(0, 0), ScreenPos(0, 0));
            if (!title.empty()) {
                BitmapCanvas measureCanvas(0, 0);
                measureCanvas.setFont(_style->getTitleFontName(), titleFontSize);
                titleSize = measureCanvas.measureTextSize(title, maxTitleWidth, _style->isTitleWrap());
            }
        
            ScreenBounds descSize(ScreenPos(0, 0), ScreenPos(0, 0));
            if (!desc.empty()) {
                BitmapCanvas measureCanvas(0, 0);
                measureCanvas.setFont(_style->getDescriptionFontName(), descFontSize);
                descSize = measureCanvas.measureTextSize(desc, maxDescWidth, _style->isDescriptionWrap());
            }

            // Measure button sizes, generate button positions
            std::map<std::shared_ptr<BalloonPopupButton>, ScreenBounds> buttonSizes;

            int buttonMarginWidth = 0;
            int buttonMarginHeight = 0;
            ScreenBounds buttonsSize(ScreenPos(0, 0), ScreenPos(0, 0));
            if (!_buttons.empty()) {
                int buttonY = 0;
                for (const std::shared_ptr<BalloonPopupButton>& button : _buttons) {
                    ScreenBounds buttonSize = measureButtonSize(button, dpToPX);
                    buttonSize.setMin(ScreenPos(buttonSize.getMin().getX() + buttonMargins.getLeft(), buttonSize.getMin().getY() + buttonMargins.getTop() + buttonY));
                    buttonSize.setMax(ScreenPos(buttonSize.getMax().getX() + buttonMargins.getLeft(), buttonSize.getMax().getY() + buttonMargins.getTop() + buttonY));
                    buttonSizes[button] = buttonSize;

                    buttonY += buttonMargins.getTop() + buttonSize.getHeight() + buttonMargins.getBottom();
                    buttonsSize.expandToContain(ScreenPos(buttonMargins.getLeft() + buttonSize.getWidth() + buttonMargins.getRight(), buttonY));
                }
            }
        
            // Calculate triangle height with stroke
            float halfTriangleWidth = triangleWidth * 0.5f;
            float halfTriangleAngle = std::atan2(triangleWidth, (triangleHeight * 2));
            int triangleStrokeOffset = static_cast<int>(triangleHeight + 2 * std::cos(halfTriangleAngle) * strokeWidth * 0.5f / std::cos(Const::PI / 2 - 2 * halfTriangleAngle) + 0.5f);
        
            // Calculate bitmap size and create canvas
            int popupInnerWidth = static_cast<int>(std::max(buttonsSize.getWidth() + buttonMarginWidth, std::max(titleSize.getWidth() + titleMarginWidth, descSize.getWidth() + descMarginWidth)));
            int popupWidth = popupInnerWidth + static_cast<int>(leftMarginWidth + rightMarginWidth + strokeWidth);
            float halfPopupWidth = popupWidth * 0.5f;
            int popupInnerHeight = static_cast<int>(std::max((float) (titleSize.getHeight() + titleMarginHeight + descSize.getHeight() + descMarginHeight + buttonsSize.getHeight() + buttonMarginHeight), (float) std::max(leftMarginHeight, rightMarginHeight)));
            int popupHeight = popupInnerHeight + static_cast<int>(std::max((float) triangleStrokeOffset, halfStrokeWidth) + halfStrokeWidth);

            int canvasWidth = popupWidth;
            int canvasHeight = popupHeight;
            if (canvasWidth > MAX_CANVAS_SIZE || canvasHeight > MAX_CANVAS_SIZE) {
                Log::Errorf("BalloonPopup::drawBitmap: Popup too large: %d x %d!", canvasWidth, canvasHeight);
                return std::shared_ptr<Bitmap>();
            }
        
            BitmapCanvas canvas(canvasWidth, canvasHeight);
        
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
        
            int maxHalfOffsetX = static_cast<int>(halfPopupWidth - halfTriangleWidth - _style->getCornerRadius() - halfStrokeWidth);
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
            if (!title.empty()) {
                ScreenPos titlePos(halfStrokeWidth + leftMarginWidth + titleMargins.getLeft(),
                                   halfStrokeWidth + titleMargins.getTop());
                canvas.setColor(_style->getTitleColor());
                canvas.setFont(_style->getTitleFontName(), titleFontSize);
                canvas.drawText(title, titlePos, titleSize.getWidth(), _style->isTitleWrap());
            }
        
            // Draw description
            if (!desc.empty()) {
                ScreenPos descPos(halfStrokeWidth + leftMarginWidth + descMargins.getLeft(),
                                  halfStrokeWidth + titleSize.getHeight() + titleMarginHeight + descMargins.getTop());
                canvas.setColor(_style->getDescriptionColor());
                canvas.setFont(_style->getDescriptionFontName(), descFontSize);
                canvas.drawText(desc, descPos, descSize.getWidth(), _style->isDescriptionWrap());
            }

            // Draw buttons, finalize button positions
            float buttonsOriginX = halfStrokeWidth + leftMarginWidth + popupInnerWidth * 0.5f;
            float buttonsOriginY = halfStrokeWidth + titleSize.getHeight() + titleMarginHeight + descSize.getHeight() + descMarginHeight;
            for (const std::shared_ptr<BalloonPopupButton>& button : _buttons) {
                const ScreenBounds& buttonSize = buttonSizes[button];
                ScreenBounds buttonRect(ScreenPos(buttonsOriginX + buttonSize.getMin().getX() - buttonSize.getWidth() * 0.5f, buttonsOriginY + buttonSize.getMin().getY()),
                                        ScreenPos(buttonsOriginX + buttonSize.getMax().getX() - buttonSize.getWidth() * 0.5f, buttonsOriginY + buttonSize.getMax().getY()));
                drawButtonOnCanvas(button, canvas, buttonRect, dpToPX);
                _buttonRects[button] = buttonRect;
            }

            // Done with internal state, update anchor point and build bitmap
            lock.unlock();
            setAnchorPoint(triangleOffsetX / halfPopupWidth, -1);

            return canvas.buildBitmap();
        }
        catch (const std::exception& ex) {
            Log::Errorf("BalloonPopup::drawBitmap: Failed to render bitmap: %s", ex.what());
            return std::shared_ptr<Bitmap>();
        }
    }

    ScreenBounds BalloonPopup::measureButtonSize(const std::shared_ptr<BalloonPopupButton>& button, float dpToPX) const {
        const std::shared_ptr<BalloonPopupButtonStyle>& buttonStyle = button->getStyle();

        int strokeWidth = buttonStyle->getStrokeWidth() * dpToPX;
        int textFontSize = buttonStyle->getTextFontSize() * dpToPX;
        BalloonPopupMargins textMargins(buttonStyle->getTextMargins().getLeft() * dpToPX, buttonStyle->getTextMargins().getTop() * dpToPX,
                                        buttonStyle->getTextMargins().getRight() * dpToPX, buttonStyle->getTextMargins().getBottom() * dpToPX);
        int maxTextWidth = buttonStyle->getButtonWidth() >= 0 ? std::max(0.0f, buttonStyle->getButtonWidth() * dpToPX - strokeWidth - textMargins.getLeft() - textMargins.getRight()) : -1;
        bool isTextWrap = buttonStyle->getButtonWidth() >= 0;

        BitmapCanvas measureCanvas(0, 0);
        measureCanvas.setFont(buttonStyle->getTextFontName(), textFontSize);
        ScreenBounds textSize = measureCanvas.measureTextSize(button->getText(), maxTextWidth, isTextWrap);

        float buttonWidth = std::max(buttonStyle->getButtonWidth() * dpToPX, strokeWidth + textMargins.getLeft() + textMargins.getRight() + textSize.getWidth());
        float buttonHeight = strokeWidth + textMargins.getTop() + textMargins.getBottom() + textSize.getHeight();
        return ScreenBounds(ScreenPos(0, 0), ScreenPos(buttonWidth, buttonHeight));
    }

    void BalloonPopup::drawButtonOnCanvas(const std::shared_ptr<BalloonPopupButton>& button, BitmapCanvas& canvas, const ScreenBounds& bounds, float dpToPX) const {
        const std::shared_ptr<BalloonPopupButtonStyle>& buttonStyle = button->getStyle();

        int strokeWidth = buttonStyle->getStrokeWidth() * dpToPX;
        int textFontSize = buttonStyle->getTextFontSize() * dpToPX;
        BalloonPopupMargins textMargins(buttonStyle->getTextMargins().getLeft() * dpToPX, buttonStyle->getTextMargins().getTop() * dpToPX,
                                        buttonStyle->getTextMargins().getRight() * dpToPX, buttonStyle->getTextMargins().getBottom() * dpToPX);
        int maxTextWidth = buttonStyle->getButtonWidth() >= 0 ? std::max(0.0f, buttonStyle->getButtonWidth() * dpToPX - strokeWidth - textMargins.getLeft() - textMargins.getRight()) : -1;
        bool isTextWrap = buttonStyle->getButtonWidth() >= 0;

        // Stroke background and triangle
        canvas.setDrawMode(BitmapCanvas::STROKE);
        canvas.setColor(buttonStyle->getStrokeColor());
        canvas.setStrokeWidth(strokeWidth);
        canvas.drawRoundRect(bounds, buttonStyle->getCornerRadius());
    
        // Fill background/2 and triangle
        canvas.setDrawMode(BitmapCanvas::FILL);
        canvas.setColor(buttonStyle->getBackgroundColor());
        canvas.drawRoundRect(bounds, buttonStyle->getCornerRadius());

        // Draw text
        ScreenPos textPos(bounds.getMin().getX() + strokeWidth * 0.5f + textMargins.getLeft(),
                          bounds.getMin().getY() + strokeWidth * 0.5f + textMargins.getTop());
        canvas.setColor(buttonStyle->getTextColor());
        canvas.setFont(buttonStyle->getTextFontName(), textFontSize);
        canvas.drawText(button->getText(), textPos, maxTextWidth, isTextWrap);
    }

    const int BalloonPopup::SCREEN_PADDING = 10;

    const int BalloonPopup::MAX_CANVAS_SIZE = 8192;
       
}
