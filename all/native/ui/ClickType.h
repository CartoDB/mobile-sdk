/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CLICKTYPE_H_
#define _CARTO_CLICKTYPE_H_

namespace carto {

    namespace ClickType {
        /**
         * Possible click types.
         */
        enum ClickType {
            /**
             * A click caused by pressing down and then releasing the screen.
             */
            CLICK_TYPE_SINGLE,
            /**
             * A click caused by pressing down but not releasing the screen.
             */
            CLICK_TYPE_LONG,
            /**
             * A click caused by two fast consecutive taps on the screen.
             */
            CLICK_TYPE_DOUBLE,
            /**
             * A click caused by two simultaneous taps on the screen.
             */
            CLICK_TYPE_DUAL
        };
    }

}

#endif
