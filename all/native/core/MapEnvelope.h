/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPENVELOPE_H_
#define _CARTO_MAPENVELOPE_H_

#include "core/MapBounds.h"

#include <memory>
#include <string>
#include <vector>

namespace carto {
    class MapPos;
    
    /**
     * A bounding area on the map. Can be defined by a map bounds object or by a convex bounding polygon.
     * This class is intended for conservative object area estimation and fast intersection testing.
     */
    class MapEnvelope {
    public:
        /**
         * Constructs an empty MapEnvelope object. Nothing is contained within
         * this envelope.
         */
        MapEnvelope();
        /**
         * Constructs a MapEnvelope object using map bounds.
         * @param bounds The bounds for the envelope.
         */
        explicit MapEnvelope(const MapBounds& bounds);
        /**
         * Constructs a MapEnvelope object from a convex bounding polygon.
         * @param convexHull The list of convex hull points.
         */
        explicit MapEnvelope(const std::vector<MapPos>& convexHull);
        virtual ~MapEnvelope();
    
        /**
         * Checks for equality between this and another envelope.
         * @param envelope The other map envelope object.
         * @return True if equal.
         */
        bool operator==(const MapEnvelope& envelope) const;
        /**
         * Checks for inequality between this and another map envelope.
         * @param envelope The other map envelope object.
         * @return True if not equal.
         */
        bool operator!=(const MapEnvelope& envelope) const;
        
        /**
         * Returns the map bounds of this map envelope.
         * @return The map bounds of this map envelope.
         */
        const MapBounds& getBounds() const;
    
        /**
         * Returns the convex hull of this map envelope.
         * @return The convex hull of this map envelope.
         */
        const std::vector<MapPos>& getConvexHull() const;
    
        /**
         * Tests whether this map envelope contains another map envelope.
         * @param envelope The other map envelope.
         * @return True if this map envelope contains the other map envelope.
         */
        bool contains(const MapEnvelope& envelope) const;
        /**
         * Tests whether this map envelope intersects with another map envelope.
         * @param envelope The other map envelope.
         * @return True if this map envelope intersects the other map envelope.
         */
        bool intersects(const MapEnvelope& envelope) const;
    
        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;

        /**
         * Creates a string representation of this map envelope, useful for logging.
         * @return The string representation of this map envelope.
         */
        std::string toString() const;
    
    private:
        MapBounds _bounds;
        bool _rectangularConvexHull;
        std::vector<MapPos> _convexHull;
    };
    
}

#endif
