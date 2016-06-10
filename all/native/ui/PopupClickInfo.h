/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POPUPCLICKINFO_H_
#define _CARTO_POPUPCLICKINFO_H_

#include "core/MapPos.h"
#include "core/ScreenPos.h"
#include "ui/ClickType.h"
#include "vectorelements/Popup.h"

#include <memory>

namespace carto {
    
    /**
     * A container class that provides information about a click performed on
     * a popup.
     */
    class PopupClickInfo {
    public:
        /**
         * Constructs a PopupClickInfo object from a click position and a vector element.
         * @param clickType The click type (SINGLE, DUAL, etc)
         * @param clickPos The click position in the coordinate system of the base projection.
         * @param elementClickPos The 2D click position on the popup.
         * @param popup The popup on which the click was performed.
         */
        PopupClickInfo(ClickType::ClickType clickType, const MapPos& clickPos, const ScreenPos& elementClickPos, const std::shared_ptr<Popup>& popup);
        virtual ~PopupClickInfo();
    
        /**
         * Returns the click type.
         * @return The type of the click performed.
         */
        ClickType::ClickType getClickType() const;

        /**
         * Returns the click position.
         * @return The click position in the coordinate system of the base projection.
         */
        const MapPos& getClickPos() const;
        
        /**
         * Returns the 2D click position on the clicked popup.
         * @return The 2D element click position.
         */
        const ScreenPos& getElementClickPos() const;
    
        /**
         * Returns the clicked popup.
         * @return The popup on which the click was performed.
         */
        std::shared_ptr<Popup> getPopup() const;

    private:
        ClickType::ClickType _clickType;
        MapPos _clickPos;
        ScreenPos _elementClickPos;
        std::shared_ptr<Popup> _popup;
    };
    
}

#endif
