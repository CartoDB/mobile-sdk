/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CLICKINFO_H_
#define _CARTO_CLICKINFO_H_

#include <string>

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

    /**
     * A basic click information object. Contains information about click type, duration.
     */
    class ClickInfo {
    public:
        /**
         * Constructs a ClickInfo object from a click type and click duration.
         * @param clickType The click type.
         * @param duration The click duration (in seconds).
         */
        ClickInfo(ClickType::ClickType clickType, float duration);
    
        /**
         * Returns the click type.
         * @return The type of the click performed.
         */
        ClickType::ClickType getClickType() const;

        /**
         * Returns the click duration in seconds.
         * @return The click duration in seconds.
         */
        float getDuration() const;

        /**
         * Checks for equality between this and another click info.
         * @param clickInfo The other click info.
         * @return True if equal.
         */
        bool operator ==(const ClickInfo& clickInfo) const;
        /**
         * Checks for inequality between this and another click info.
         * @param clickInfo The other click info.
         * @return True if not equal.
         */
        bool operator !=(const ClickInfo& clickInfo) const;

        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;

        /**
         * Creates a string representation of this click info, useful for logging.
         * @return The string representation of this click info.
         */
        std::string toString() const;

   private:
        ClickType::ClickType _clickType;
        float _duration;
   };

}

#endif
