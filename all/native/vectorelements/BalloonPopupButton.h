/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BALLOONPOPUPBUTTON_H_
#define _CARTO_BALLOONPOPUPBUTTON_H_

#include "core/Variant.h"

#include <memory>
#include <mutex>
#include <string>

namespace carto {
    class BalloonPopupButtonStyle;

    /**
     * A button that can be used to add interactivity to balloon popups.
     */
    class BalloonPopupButton : public std::enable_shared_from_this<BalloonPopupButton> {
    public:
        /**
         * Constructs a BalloonPopupButton object with the specified style and text.
         * @param style The style that defines what this button looks like.
         * @param text The text for this button.
         */
        BalloonPopupButton(const std::shared_ptr<BalloonPopupButtonStyle>& style, const std::string& text);
        virtual ~BalloonPopupButton();
        
        /**
         * Returns the text of this button.
         * @return The text of this button.
         */
        std::string getText() const;
        
        /**
         * Returns the style of this button.
         * @return The style that defines what this button looks like.
         */
        std::shared_ptr<BalloonPopupButtonStyle> getStyle() const;

        /**
         * Returns the user-defined tag associated with the button.
         * @return The user-defined tag value. By default the tag is empty.
         */
        Variant getTag() const;
        /**
         * Sets the user-defined tag associated with the button.
         * @param tag The new user-defined tag value.
         */
        void setTag(const Variant& tag);

    private:
        std::shared_ptr<BalloonPopupButtonStyle> _style;
        
        std::string _text;

        Variant _tag;

        mutable std::mutex _mutex;
    };

}

#endif
