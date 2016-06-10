/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POPUPDRAWINFO_H_
#define _CARTO_POPUPDRAWINFO_H_

#include "core/ScreenPos.h"
#include "core/ScreenBounds.h"
#include "vectorelements/Popup.h"

#include <memory>

namespace carto {
    
    /**
     * A container class that provides information how to draw the popup.
     */
    class PopupDrawInfo {
    public:
        /**
         * Constructs a PopupClickInfo object from a click position and a vector element.
         * @param anchorPoint The anchor point of the popup.
         * @param screenBounds The screen bounds for clipping the popup.
         * @param popup The popup on which the click was performed.
         * @param dpToPX The value used for converting display independent pixels (dp) to pixels (px).
         */
        PopupDrawInfo(const ScreenPos& anchorPoint, const ScreenBounds& screenBounds, const std::shared_ptr<Popup>& popup, float dpToPX);
        virtual ~PopupDrawInfo();
    
        /**
         * Returns the popup anchor point coordinates.
         * @return The popup anchor point coordinates.
         */
        const ScreenPos& getAnchorPoint() const;
        
        /**
         * Returns the screen bounds, so that the popup can be clipped if neccessary.
         * @return The screen bounds in pixels.
         */
        const ScreenBounds& getScreenBounds() const;
    
        /**
         * Returns the popup to draw.
         * @return The popup to draw.
         */
        std::shared_ptr<Popup> getPopup() const;

        /**
         * Returns the value used for converting display independent pixels (dp) to pixels (px).
         * @return The value used for converting display independent pixels (dp) to pixels (px).
         */
        float getDPToPX() const;
    
    private:
        ScreenPos _anchorPoint;
        ScreenBounds _screenBounds;
        std::shared_ptr<Popup> _popup;
        float _dpToPX;
    };
    
}

#endif
