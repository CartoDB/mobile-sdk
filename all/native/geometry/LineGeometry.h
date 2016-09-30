/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LINEGEOMETRY_H_
#define _CARTO_LINEGEOMETRY_H_

#include "geometry/Geometry.h"

#include <vector>

namespace carto {

    /**
     * Line geometry defined by a list of map positions.
     */
    class LineGeometry : public Geometry {
    public:
        /**
         * Constructs a new LineGeometry object from a map position list.
         * @param poses The map position list.
         */
        explicit LineGeometry(const std::vector<MapPos>& poses);
        virtual ~LineGeometry();
        
        virtual MapPos getCenterPos() const;
    
        /**
         * Returns the list of of map positions defining the line.
         * @return The list of of map positions defining the line.
         */
        const std::vector<MapPos>& getPoses() const;
    
    private:
        std::vector<MapPos> _poses;
    };
    
}

#endif
