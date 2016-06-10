#include "PopupClickInfo.h"

namespace carto {

    PopupClickInfo::PopupClickInfo(ClickType::ClickType clickType, const MapPos& clickPos, const ScreenPos& elementClickPos,
                                                   const std::shared_ptr<Popup>& popup) :
        _clickType(clickType),
        _clickPos(clickPos),
        _elementClickPos(elementClickPos),
        _popup(popup)
    {
    }
    
    PopupClickInfo::~PopupClickInfo() {
    }

    ClickType::ClickType PopupClickInfo::getClickType() const {
        return _clickType;
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
