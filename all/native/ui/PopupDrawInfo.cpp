#include "PopupDrawInfo.h"

namespace carto {

    PopupDrawInfo::PopupDrawInfo(const ScreenPos& anchorScreenPos, const ScreenBounds& screenBounds, const std::shared_ptr<Popup>& popup, float dpToPX) :
        _anchorScreenPos(anchorScreenPos),
        _screenBounds(screenBounds),
        _popup(popup),
        _dpToPX(dpToPX)
    {
    }
    
    PopupDrawInfo::~PopupDrawInfo() {
    }

    const ScreenPos& PopupDrawInfo::getAnchorScreenPos() const {
        return _anchorScreenPos;
    }
        
    const ScreenBounds& PopupDrawInfo::getScreenBounds() const {
        return _screenBounds;
    }
    
    std::shared_ptr<Popup> PopupDrawInfo::getPopup() const {
        return _popup;
    }
    
    float PopupDrawInfo::getDPToPX() const {
        return _dpToPX;
    }

}
