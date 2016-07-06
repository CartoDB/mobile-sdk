/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOMETRY_H_
#define _CARTO_GEOMETRY_H_

#include "core/MapPos.h"
#include "core/MapBounds.h"

#include <string>

namespace carto {

    /**
     * A base class for all geometry types.
     */
    class Geometry {
    public:
        virtual ~Geometry() { }
        
        /**
         * Returns the center point of the geometry.
         * @return The center point of the geometry.
         */
        virtual MapPos getCenterPos() const = 0;
    
        /**
         * Returns the minimal bounds for the geometry.
         * @return The bounds for the geometry.
         */
        const MapBounds& getBounds() const {
            return _bounds;
        }
    
    protected:
        Geometry() : _bounds() { }
    
        MapBounds _bounds;
    };
    
}

#endif
