/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CUSTOMPOPUP_H_
#define _CARTO_CUSTOMPOPUP_H_

#include "components/DirectorPtr.h"
#include "core/MapPos.h"
#include "core/ScreenPos.h"
#include "ui/ClickType.h"
#include "vectorelements/Popup.h"

namespace carto {
    class CustomPopupHandler;
    
    /**
     * A popup with user defined handler. The handler is responsible
     * for drawing the bitmap for the popup and can also respond to
     * touch event.
     */
    class CustomPopup : public Popup {
    public:
        /**
         * Constructs a CustomPopup object with the specified style and attaches it to a billboard element.
         * @param baseBillboard The billboard this popup will be attached to.
         * @param style The style that defines what this popup looks like.
         * @param popupHandler The handler to use for the popup.
         */
        CustomPopup(const std::shared_ptr<Billboard>& baseBillboard, const std::shared_ptr<PopupStyle>& style, const std::shared_ptr<CustomPopupHandler>& popupHandler);
        /**
         * Constructs a CustomPopup object from a geometry object and a style.
         * @param geometry The geometry object that defines the location of this popup.
         * @param style The style that defines what this popup looks like.
         * @param popupHandler The handler to use for the popup.
         */
        CustomPopup(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<PopupStyle>& style, const std::shared_ptr<CustomPopupHandler>& popupHandler);
        /**
         * Constructs a CustomPopup object from a map position and a style.
         * @param pos The map position that defines the location of this popup.
         * @param style The style that defines what this popup looks like.
         * @param popupHandler The handler to use for the popup.
         */
        CustomPopup(const MapPos& pos, const std::shared_ptr<PopupStyle>& style, const std::shared_ptr<CustomPopupHandler>& popupHandler);
        virtual ~CustomPopup();

        /**
         * Returns the handler used for the popup.
         * @return The popup handler responsible for drawing the popup and handling click events.
         */
        std::shared_ptr<CustomPopupHandler> getPopupHandler() const;
    
        /**
         * Handles the click event for this Popup.
         * @param clickType The type of the click.
         * @param clickPos The position of the click.
         * @param elementClickPos The 2D position of the click on the popup.
         * @return True if the click was handled. False otherwise.
         */
        virtual bool processClick(ClickType::ClickType clickType, const MapPos& clickPos, const ScreenPos& elementClickPos);
        
        virtual std::shared_ptr<Bitmap> drawBitmap(const ScreenPos& anchorScreenPos,
                                                    float screenWidth, float screenHeight, float dpToPX);

    private:
        const DirectorPtr<CustomPopupHandler> _popupHandler;
    };
    
}

#endif
