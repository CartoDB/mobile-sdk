#include "BalloonPopupButtonClickInfo.h"

namespace carto {

    BalloonPopupButtonClickInfo::BalloonPopupButtonClickInfo(ClickType::ClickType clickType,
                                                   const std::shared_ptr<BalloonPopupButton>& button,
                                                   const std::shared_ptr<BalloonPopup>& balloonPopup) :
        _clickType(clickType),
        _button(button),
        _balloonPopup(balloonPopup)
    {
    }
    
    BalloonPopupButtonClickInfo::~BalloonPopupButtonClickInfo() {
    }

    ClickType::ClickType BalloonPopupButtonClickInfo::getClickType() const {
        return _clickType;
    }
    
    std::shared_ptr<BalloonPopupButton> BalloonPopupButtonClickInfo::getButton() const {
        return _button;
    }
    
    std::shared_ptr<BalloonPopup> BalloonPopupButtonClickInfo::getBalloonPopup() const {
        return _balloonPopup;
    }
    
}
