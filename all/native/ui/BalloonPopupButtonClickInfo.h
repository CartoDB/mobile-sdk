/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BALLOONPOPUPBUTTONCLICKINFO_H_
#define _CARTO_BALLOONPOPUPBUTTONCLICKINFO_H_

#include "ui/ClickType.h"
#include "vectorelements/BalloonPopupButton.h"

#include <memory>

namespace carto {
    class BalloonPopup;
    class Layer;
    
    /**
     * A container class that provides information about a click performed on
     * a BalloonPopupButton element.
     */
    class BalloonPopupButtonClickInfo {
    public:
        /**
         * Constructs a BalloonPopupButtonClickInfo object from a click type and a button element.
         * @param clickType The click type (SINGLE, DUAL, etc)
         * @param button The button on which the click was performed.
         * @param balloonPopup The balloon popup on which the click was performed.
         */
        BalloonPopupButtonClickInfo(ClickType::ClickType clickType, const std::shared_ptr<BalloonPopupButton>& button, const std::shared_ptr<BalloonPopup>& balloonPopup);
        virtual ~BalloonPopupButtonClickInfo();
    
        /**
         * Returns the click type.
         * @return The type of the click performed.
         */
        ClickType::ClickType getClickType() const;

        /**
         * Returns the clicked button.
         * @return The button on which the click was performed.
         */
        std::shared_ptr<BalloonPopupButton> getButton() const;

        /**
         * Returns the clicked ballon popup.
         * @return The balloon popup on which the click was performed.
         */
        std::shared_ptr<BalloonPopup> getBalloonPopup() const;

    private:
        ClickType::ClickType _clickType;
    
        std::shared_ptr<BalloonPopupButton> _button;
        std::shared_ptr<BalloonPopup> _balloonPopup;
    };
    
}

#endif
