#include "BalloonPopupButtonClickInfo.h"
#include "vectorelements/BalloonPopupButton.h"
#include "vectorelements/VectorElement.h"

namespace carto {

    BalloonPopupButtonClickInfo::BalloonPopupButtonClickInfo(const ClickInfo& clickInfo,
                                                   const std::shared_ptr<BalloonPopupButton>& button,
                                                   const std::shared_ptr<VectorElement>& vectorElement) :
        _clickInfo(clickInfo),
        _button(button),
        _vectorElement(vectorElement)
    {
    }
    
    BalloonPopupButtonClickInfo::~BalloonPopupButtonClickInfo() {
    }

    ClickType::ClickType BalloonPopupButtonClickInfo::getClickType() const {
        return _clickInfo.getClickType();
    }
    
    const ClickInfo& BalloonPopupButtonClickInfo::getClickInfo() const {
        return _clickInfo;
    }
    
    std::shared_ptr<BalloonPopupButton> BalloonPopupButtonClickInfo::getButton() const {
        return _button;
    }
    
    std::shared_ptr<VectorElement> BalloonPopupButtonClickInfo::getVectorElement() const {
        return _vectorElement;
    }
    
}
