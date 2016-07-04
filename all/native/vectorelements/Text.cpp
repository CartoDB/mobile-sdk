#include "Text.h"
#include "graphics/Bitmap.h"
#include "graphics/BitmapCanvas.h"
#include "styles/TextStyle.h"
#include "utils/Const.h"

#include <cstdlib>
#include <cmath>

namespace carto {
    
    Text::Text(const std::shared_ptr<Billboard>& baseBillboard, const std::shared_ptr<TextStyle>& style, const std::string& text) :
    	Label(baseBillboard, style),
    	_style(style),
    	_text(text)
    {
    }

    Text::Text(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<TextStyle>& style,
                         const std::string& text) :
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
        std::lock_guard<std::mutex> lock(_mutex);

        // Scale with DPI, if necessary
        if (_style->isScaleWithDPI()) {
            dpToPX = 1;
        }
        
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

        BitmapCanvas measureCanvas(0, 0);
        measureCanvas.setFont(_style->getFontName(), fontSize);
        ScreenBounds textBounds = measureCanvas.measureTextSize(text, -1, false);

        BitmapCanvas canvas(static_cast<int>(std::ceil(textBounds.getWidth() + strokeWidth)), static_cast<int>(std::ceil(textBounds.getHeight() + strokeWidth)));
        canvas.setFont(_style->getFontName(), fontSize);

        if (strokeWidth > 0) {
            canvas.setColor(_style->getStrokeColor());
            canvas.setDrawMode(BitmapCanvas::STROKE);
            canvas.setStrokeWidth(strokeWidth);
            canvas.drawText(text, ScreenPos(strokeWidth * 0.5f, strokeWidth * 0.5f), textBounds.getWidth(), false);
        }

        canvas.setColor(_style->getFontColor());
        canvas.setDrawMode(BitmapCanvas::FILL);
        canvas.drawText(text, ScreenPos(strokeWidth * 0.5f, strokeWidth * 0.5f), textBounds.getWidth(), false);

        return canvas.buildBitmap();
    }
        
    std::string Text::getText() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _text;
    }

    void Text::setText(const std::string& text) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _text = text;
        }
        notifyElementChanged();
    }
        
    std::shared_ptr<TextStyle> Text::getStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _style;
    }

    void Text::setStyle(const std::shared_ptr<TextStyle>& style) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _style = style;
        }
        Label::setStyle(style);
    }

}
