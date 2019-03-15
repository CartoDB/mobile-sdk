#include "Text.h"
#include "components/Exceptions.h"
#include "graphics/Bitmap.h"
#include "graphics/BitmapCanvas.h"
#include "styles/TextStyle.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cstdlib>
#include <cmath>

namespace carto {
    
    Text::Text(const std::shared_ptr<Billboard>& baseBillboard, const std::shared_ptr<TextStyle>& style, const std::string& text) :
        Label(baseBillboard, style),
        _style(style),
        _text(text)
    {
    }

    Text::Text(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<TextStyle>& style, const std::string& text) :
        Label(geometry, style),
        _style(style),
        _text(text)
    {
    }
        
    Text::Text(const MapPos& pos, const std::shared_ptr<TextStyle>& style, const std::string& text) :
        Label(pos, style),
        _style(style),
        _text(text)
    {
    }

    Text::~Text() {
    }
        
    std::shared_ptr<Bitmap> Text::drawBitmap(float dpToPX) const {
        try {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            // Scale with DPI, if necessary
            if (_style->isScaleWithDPI()) {
                dpToPX = 1;
            }

            // Multiply with rendering scale
            dpToPX *= _style->getRenderScale();
        
            // Use actual text or text field
            std::string text = _text;
            if (text.empty() && !_style->getTextField().empty()) {
                Variant value = getMetaDataElement(_style->getTextField());
                if (value.getType() == VariantType::VARIANT_TYPE_STRING) {
                    text = value.getString();
                } else {
                    text = value.toString();
                }
            }

            float fontSize = _style->getFontSize() * dpToPX;
            float strokeWidth = _style->getStrokeWidth() * dpToPX;
            float borderWidth = _style->getBorderWidth() * dpToPX;
            float leftPadding = _style->getTextMargins().getLeft() * dpToPX;
            float rightPadding = _style->getTextMargins().getRight() * dpToPX;
            float topPadding = _style->getTextMargins().getTop() * dpToPX;
            float bottomPadding = _style->getTextMargins().getBottom() * dpToPX;
            float borderPadding = (borderWidth > 0 ? 1 : 0);

            BitmapCanvas measureCanvas(0, 0);
            measureCanvas.setFont(_style->getFontName(), fontSize);
            ScreenBounds textBounds = measureCanvas.measureTextSize(text, -1, _style->isBreakLines());

            int canvasWidth = static_cast<int>(std::ceil(textBounds.getWidth() + strokeWidth + leftPadding + rightPadding + 2 * borderWidth + 2 * borderPadding));
            int canvasHeight = static_cast<int>(std::ceil(textBounds.getHeight() + strokeWidth + topPadding + bottomPadding + 2 * borderWidth + 2 * borderPadding));
            if (canvasWidth > MAX_CANVAS_SIZE || canvasHeight > MAX_CANVAS_SIZE) {
                Log::Errorf("Text::drawBitmap: Text too large: %d x %d!", canvasWidth, canvasHeight);
                return std::shared_ptr<Bitmap>();
            }

            BitmapCanvas canvas(canvasWidth, canvasHeight);
            canvas.setFont(_style->getFontName(), fontSize);

            if (_style->getBackgroundColor() != Color()) {
                canvas.setColor(_style->getBackgroundColor());
                canvas.setDrawMode(BitmapCanvas::FILL);
                canvas.drawRoundRect(ScreenBounds(ScreenPos(borderPadding, borderPadding), ScreenPos(canvasWidth - borderPadding, canvasHeight - borderPadding)), 0);
            }

            if (borderWidth > 0 && _style->getBorderColor() != Color()) {
                canvas.setColor(_style->getBorderColor());
                canvas.setDrawMode(BitmapCanvas::STROKE);
                canvas.setStrokeWidth(borderWidth);
                canvas.drawRoundRect(ScreenBounds(ScreenPos(0.5f * borderWidth + borderPadding, 0.5f * borderWidth + borderPadding), ScreenPos(canvasWidth - borderPadding - 0.5f * borderWidth, canvasHeight - borderPadding - 0.5f * borderWidth)), 0);
            }

            if (strokeWidth > 0) {
                canvas.setColor(_style->getStrokeColor());
                canvas.setDrawMode(BitmapCanvas::STROKE);
                canvas.setStrokeWidth(strokeWidth);
                canvas.drawText(text, ScreenPos(borderPadding + borderWidth + leftPadding + strokeWidth * 0.5f, borderPadding + borderWidth + topPadding + strokeWidth * 0.5f), textBounds.getWidth(), _style->isBreakLines());
            }

            canvas.setColor(_style->getFontColor());
            canvas.setDrawMode(BitmapCanvas::FILL);
            canvas.drawText(text, ScreenPos(borderPadding + borderWidth + leftPadding + strokeWidth * 0.5f, borderPadding + borderWidth + topPadding + strokeWidth * 0.5f), textBounds.getWidth(), _style->isBreakLines());

            return canvas.buildBitmap();
        }
        catch (const std::exception& ex) {
            Log::Errorf("Text::drawBitmap: Failed to render bitmap: %s", ex.what());
            return std::shared_ptr<Bitmap>();
        }
    }
        
    std::string Text::getText() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _text;
    }

    void Text::setText(const std::string& text) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _text = text;
        }
        notifyElementChanged();
    }
        
    std::shared_ptr<TextStyle> Text::getStyle() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _style;
    }

    void Text::setStyle(const std::shared_ptr<TextStyle>& style) {
        if (!style) {
            throw NullArgumentException("Null style");
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _style = style;
        }
        Label::setStyle(style);
    }

}
