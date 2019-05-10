/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BALLOONPOPUPEVENTLISTENER_H_
#define _CARTO_BALLOONPOPUPEVENTLISTENER_H_

#include <memory>

namespace carto {
    class BalloonPopupButtonClickInfo;
    
    /**
     * A listener class for balloon popups.
     * The listener receives events about clicked buttons, etc.
     */
    class BalloonPopupEventListener {
    public:
        virtual ~BalloonPopupEventListener() { }

        /**
         * Listener method that gets called when a clicked is performed on a button.
         * @param clickInfo The information about the button click event.
         * @return True if the click was handled. False otherwise.
         */
        virtual bool onButtonClicked(const std::shared_ptr<BalloonPopupButtonClickInfo>& clickInfo) { return true; }
    };
    
}

#endif
