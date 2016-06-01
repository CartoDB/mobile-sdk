/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPBOUNDS_H_
#define _CARTO_MAPBOUNDS_H_

#include "core/MapPos.h"
#include "core/MapVec.h"

#include <string>

namespace carto {

    /**
     * A container class that defines an axis aligned cuboid on the map using minimum and maximum map positions.
     * Valid ranges for map bounds depend on the projection used.
     */
    class MapBounds {
    public:
        /**
         * Constructs an empty MapBounds object. The coordinates of the minimum map position will be 
         * set to positive infinity and the coordinates of the maximum map position will be
         * set to negative infinity.
         */
        MapBounds();
        /**
         * Constructs a MapBounds object from a minimum and maximum map position. If a coordinate of the 
         * minimum map positon is larger than the same coordinate of the maximum map position then those
         * coordinates will be swapped.
         * @param min The minimum map position (south-west).
         * @param max The maximum map position (north-east).
         */
        MapBounds(const MapPos& min, const MapPos& max);
        virtual ~MapBounds();
    
        /**
         * Checks for equality between this and another map bounds object.
         * @param mapBounds The other map bounds object.
         * @return True if equal.
         */
        bool operator==(const MapBounds& mapBounds) const;
        /**
         * Checks for inequality between this and another map bounds object.
         * @param mapBounds The other map bounds object.
         * @return True if not equal.
         */
        bool operator!=(const MapBounds& mapBounds) const;
        
        /**
         * Sets the minimum and maximum map positions of this map bounds object. If a coordinate of the
         * minimum map positon is larger than the same coordinate of the maximum map position then those
         * coordinates will be swapped.
         * @param min The minimum map position (south-west).
         * @param max The maximum map position (north-east).
         */
        void setBounds(const MapPos& min, const MapPos& max);
        
        /**
         * Calculates the center map position of this map envelope object.
         * @return The center postion if this map envelope object.
         */
        MapPos getCenter() const;
        
        /**
         * Calculates the difference vector between the maximum and minimum map positions of this map bounds object.
         * @return The difference vector between maximum and minimum map positions of this map bounds object.
         */
        MapVec getDelta() const;
    
        /**
         * Returns the minimum (south west) map position of this map envelope object.
         * @return The minimum (south west) map position of this map envelope object.
         */
        const MapPos& getMin() const;
        /**
         * Sets the minimum (south west) map position of this map envelope object.
         * @param min The new minimum (south west) map position of this map envelope object.
         */
        void setMin(const MapPos& min);
    
        /**
         * Returns the maximum (north east) map position of this map envelope object.
         * @return The maximum (north east) map position of this map envelope object.
         */
        const MapPos& getMax() const;
        /**
         * Sets the maximum (north east) map position of this map envelope object.
         * @param max The new maximum (north east) map position of this map envelope object.
         */
        void setMax(const MapPos& max);
    
        /**
         * Tests whether this map bounds object contains a map position.
         * @param pos The map position.
         * @return True if this map bounds object contains the map position.
         */
        bool contains(const MapPos& pos) const;
        /**
         * Tests whether this map bounds object contains a another map bounds object.
         * @param bounds The other map bounds object.
         * @return True if this map bounds object contains the other map bounds object.
         */
        bool contains(const MapBounds& bounds) const;
        /**
         * Tests whether this map bounds object intersects with a another map bounds object.
         * @param bounds The other map bounds object.
         * @return True if this map bounds object intersects with the other map bounds object.
         */
        bool intersects(const MapBounds& bounds) const;
        
        /**
         * Expands this map bounds object to contain a map position.
         * @param pos The map position.
         */
        void expandToContain(const MapPos& pos);
        /**
         * Expands this map bounds object to contain another map bounds object.
         * @param bounds The other map bounds object.
         */
        void expandToContain(const MapBounds& bounds);
        
        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;
        
        /**
         * Creates a string representation of this map bounds object, useful for logging.
         * @return The string representation of this map bounds object.
         */
        std::string toString() const;
        
    private:
        MapPos _min;
        MapPos _max;
    };
    
}

#endif
