/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPRANGE_H_
#define _CARTO_MAPRANGE_H_

#include <string>

namespace carto {

    /**
     * A container class that defines a half closed range of values using minimum and maximum values.
     */
    class MapRange {
    public:
        /**
         * Constructs a MapRange object. The minimum value will be set to positive infinity
         * and the maximum value to negative infinity.
         */
        MapRange();
        /**
         * Constructs a MapRange object from min and max values.
         * If min > max, the values will be swapped internally.
         * @param min The min value.
         * @param max The max value.
         */
        MapRange(float min, float max);
    
        /**
         * Checks for equality between this and another map tile.
         * @param mapRange The other map range.
         * @return True if equal.
         */
        bool operator ==(const MapRange& mapRange) const;
        /**
         * Checks for inequality between this and another map tile.
         * @param mapRange The other map range.
         * @return True if not equal.
         */
        bool operator !=(const MapRange& mapRange) const;
    
        /**
        * Returns the min value of this map range.
        * @return The min value of this map range.
        */
        float getMin() const;
        /**
         * Sets the min value of this map range.
         * @param min The new min value of this map range.
         */
        void setMin(float min);
        /**
         * Returns the max value of this map range.
         * @return The max value of this map range.
         */
        float getMax() const;
        /**
         * Sets the max value of this map range.
         * @param max The new max value of this map range.
         */
        void setMax(float max);
    
        /**
         * Tests if a value is in this map range. Value is considered in range if min <= val < max.
         * @param value The value to be tested.
         * @return True if value is in this map range.
         */
        bool inRange(float value) const;
        /**
         * Sets the min and max values of this map range.
         * @param min The new min value of this map range.
         * @param max The new max value of this map range.
         */
        void setRange(float min, float max);
    
        /**
         * Calculates the length of this map range. Defined as max - min.
         * @return The length of this map range.
         */
        float length() const;
    
        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;

        /**
         * Creates a string representation of this map range, useful for logging.
         * @return The string representation of this map range.
         */
        std::string toString() const;
        
    private:
        float _min;
        float _max;
    };
    
}

#endif
