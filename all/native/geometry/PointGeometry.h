/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POINTGEOMETRY_H_
#define _CARTO_POINTGEOMETRY_H_

#include "geometry/Geometry.h"

namespace carto {

    /**
     * Point geometry. Geometry is defined by a single map position.
     */
    class PointGeometry : public Geometry {
    public:
        /**
         * Constructs a PointGeometry object from a given map position.
         * @param pos The map position.
         */
        explicit PointGeometry(const MapPos& pos);
        virtual ~PointGeometry();
    
        virtual MapPos getCenterPos() const;
     
        /**
         * Returns the position of the point.
         * @returns The position of the point.
         */
        const MapPos& getPos() const;
    
    private:
        MapPos _pos;
    };
    
}

#endif
