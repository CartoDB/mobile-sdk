#include "PopupClickInfo.h"

namespace carto {

    PopupClickInfo::PopupClickInfo(const ClickInfo& clickInfo, const MapPos& clickPos, const ScreenPos& elementClickPos, const std::shared_ptr<Popup>& popup) :
        _clickInfo(clickInfo),
        _clickPos(clickPos),
        _elementClickPos(elementClickPos),
        _popup(popup)
    {
    }
    
    PopupClickInfo::~PopupClickInfo() {
    }

    ClickType::ClickType PopupClickInfo::getClickType() const {
        return _clickInfo.getClickType();
    }
    
    const ClickInfo& PopupClickInfo::getClickInfo() const {
        return _clickInfo;
    }

    const MapPos& PopupClickInfo::getClickPos() const {
        return _clickPos;
    }
        
    const ScreenPos& PopupClickInfo::getElementClickPos() const {
        return _elementClickPos;
    }
    
    std::shared_ptr<Popup> PopupClickInfo::getPopup() const {
        return _popup;
    }
    
}
