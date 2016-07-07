#include "CustomPopup.h"
#include "vectorelements/CustomPopupHandler.h"
#include "ui/PopupClickInfo.h"
#include "ui/PopupDrawInfo.h"

namespace carto {
    
    CustomPopup::CustomPopup(const std::shared_ptr<Billboard>& baseBillboard, const std::shared_ptr<PopupStyle>& style, const std::shared_ptr<CustomPopupHandler>& popupHandler) :
        Popup(baseBillboard, style),
        _popupHandler(popupHandler)
    {
        if (!popupHandler) {
            throw std::invalid_argument("Null popupHandler");
        }
    }
    
    CustomPopup::CustomPopup(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<PopupStyle>& style, const std::shared_ptr<CustomPopupHandler>& popupHandler) :
        Popup(geometry, style),
        _popupHandler(popupHandler)
    {
        if (!popupHandler) {
            throw std::invalid_argument("Null popupHandler");
        }
    }
    
    CustomPopup::CustomPopup(const MapPos& pos, const std::shared_ptr<PopupStyle>& style, const std::shared_ptr<CustomPopupHandler>& popupHandler) :
        Popup(pos, style),
        _popupHandler(popupHandler)
    {
        if (!popupHandler) {
            throw std::invalid_argument("Null popupHandler");
        }
    }
    
    CustomPopup::~CustomPopup() {
    }

    std::shared_ptr<CustomPopupHandler> CustomPopup::getPopupHandler() const {
        return _popupHandler.get();
    }
    
    std::shared_ptr<Bitmap> CustomPopup::drawBitmap(const ScreenPos& anchorScreenPos,
                                                    float screenWidth, float screenHeight, float dpToPX) {
        auto popupDrawInfo = std::make_shared<PopupDrawInfo>(anchorScreenPos, ScreenBounds(ScreenPos(0, 0), ScreenPos(screenWidth, screenHeight)), std::static_pointer_cast<Popup>(shared_from_this()), dpToPX);
        return _popupHandler->onDrawPopup(popupDrawInfo);
    }

    bool CustomPopup::processClick(ClickType::ClickType clickType, const MapPos& clickPos, const ScreenPos& elementClickPos) {
        auto popupClickInfo = std::make_shared<PopupClickInfo>(clickType, clickPos, elementClickPos, std::static_pointer_cast<Popup>(shared_from_this()));
        return _popupHandler->onPopupClicked(popupClickInfo);
    }

}
