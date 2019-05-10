#include "BalloonPopupButton.h"
#include "components/Exceptions.h"
#include "graphics/BitmapCanvas.h"
#include "styles/BalloonPopupButtonStyle.h"
#include "utils/Log.h"

namespace carto {
    
    BalloonPopupButton::BalloonPopupButton(const std::shared_ptr<BalloonPopupButtonStyle>& style, const std::string& text) :
        _style(style),
        _text(text),
        _tag(),
        _mutex()
    {
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }

    BalloonPopupButton::~BalloonPopupButton() {
    }
        
    std::string BalloonPopupButton::getText() const {
        return _text;
    }

    std::shared_ptr<BalloonPopupButtonStyle> BalloonPopupButton::getStyle() const {
        return _style;
    }
        
    Variant BalloonPopupButton::getTag() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _tag;
    }
        
    void BalloonPopupButton::setTag(const Variant& tag) {
        std::lock_guard<std::mutex> lock(_mutex);
        _tag = tag;
    }
        
}
