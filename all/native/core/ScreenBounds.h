/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SCREENBOUNDS_H_
#define _CARTO_SCREENBOUNDS_H_

#include "core/ScreenPos.h"

#include <string>

namespace carto {
    
    /**
     * A container class that defines a bounding box on the screen using minimum and maximum screen positions.
     */
    class ScreenBounds {
    public:
        /**
         * Constructs an empty ScreenBounds object. The coordinates of the minimum position will be
         * set to positive infinity and the coordinates of the maximum position will be
         * set to negative infinity.
         */
        ScreenBounds();
        /**
         * Constructs a ScreenBounds object from a minimum and maximum position. If a coordinate of the
         * minimum positon is larger than the same coordinate of the maximum position then those
         * coordinates will be swapped.
         * @param min The minimum position.
         * @param max The maximum position.
         */
        ScreenBounds(const ScreenPos& min, const ScreenPos& max);
        virtual ~ScreenBounds();
        
        /**
         * Checks for equality between this and another screen bounds object.
         * @param ScreenBounds The other screen bounds object.
         * @return True if equal.
         */
        bool operator==(const ScreenBounds& ScreenBounds) const;
        /**
         * Checks for inequality between this and another screen bounds object.
         * @param ScreenBounds The other screen bounds object.
         * @return True if not equal.
         */
        bool operator!=(const ScreenBounds& ScreenBounds) const;
        
        /**
         * Sets the minimum and maximum screen positions of this screen bounds object. If a coordinate of the
         * minimum screen positon is larger than the same coordinate of the maximum screen position then those
         * coordinates will be swapped.
         * @param min The minimum screen position.
         * @param max The maximum screen position.
         */
        void setBounds(const ScreenPos& min, const ScreenPos& max);
        
        /**
         * Calculates the center screen position of this screen envelope object.
         * @return The center postion if this screen envelope object.
         */
        ScreenPos getCenter() const;
        /**
         * Returns the width of the bounds object.
         * @return The width on the bounds object.
         */
        float getWidth() const;
        /**
         * Returns the height of the bounds object.
         * @return The height on the bounds object.
         */
        float getHeight() const;
        
        /**
         * Returns the minimum screen position.
         * @return The minimum screen position.
         */
        const ScreenPos& getMin() const;
        /**
         * Sets the minimum screen position.
         * @param min The new minimum screen position.
         */
        void setMin(const ScreenPos& min);
        
        /**
         * Returns the maximum screen position of this screen envelope object.
         * @return The maximum screen position of this screen envelope object.
         */
        const ScreenPos& getMax() const;
        /**
         * Sets the maximum screen position.
         * @param max The new maximum screen position.
         */
        void setMax(const ScreenPos& max);
        
        /**
         * Tests whether this screen bounds object contains a screen position.
         * @param pos The screen position.
         * @return True if this screen bounds object contains the screen position.
         */
        bool contains(const ScreenPos& pos) const;
        /**
         * Tests whether this screen bounds object contains a another screen bounds object.
         * @param bounds The other screen bounds object.
         * @return True if this screen bounds object contains the other screen bounds object.
         */
        bool contains(const ScreenBounds& bounds) const;
        /**
         * Tests whether this screen bounds object intersects with a another screen bounds object.
         * @param bounds The other screen bounds object.
         * @return True if this screen bounds object intersects with the other screen bounds object.
         */
        bool intersects(const ScreenBounds& bounds) const;
        
        /**
         * Expands this screen bounds object to contain a screen position.
         * @param pos The screen position.
         */
        void expandToContain(const ScreenPos& pos);
        /**
         * Expands this screen bounds object to contain another screen bounds object.
         * @param bounds The other screen bounds object.
         */
        void expandToContain(const ScreenBounds& bounds);
        
        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;

        /**
         * Creates a string representation of this screen bounds object, useful for logging.
         * @return The string representation of this screen bounds object.
         */
        std::string toString() const;
        
    private:
        ScreenPos _min;
        ScreenPos _max;
    };
    
}

#endif
