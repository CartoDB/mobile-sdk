/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CUSTOMPOPUPHANDLER_H_
#define _CARTO_CUSTOMPOPUPHANDLER_H_

#include <memory>

namespace carto {
    class Bitmap;
    class PopupDrawInfo;
    class PopupClickInfo;
    
    /**
     * A handler class for custom popups. The handler is responsible for drawing the popup
     * and reponding to click events.
     */
    class CustomPopupHandler {
    public:
        virtual ~CustomPopupHandler() { }

        /**
         * Handler method that gets called when the popup needs to be drawn to a bitmap.
         * @param popupDrawInfo The information about the popup to be rendered.
         * @return The drawn bitmap.
         */
        virtual std::shared_ptr<Bitmap> onDrawPopup(const std::shared_ptr<PopupDrawInfo>& popupDrawInfo) { return std::shared_ptr<Bitmap>(); }

        /**
         * Handler method that gets called when the popup is clicked
         * @param popupClickInfo The information about the popup click event.
         * @return True if the click was handler. False otherwise.
         */
        virtual bool onPopupClicked(const std::shared_ptr<PopupClickInfo>& popupClickInfo) { return true; }
    };
    
}

#endif
