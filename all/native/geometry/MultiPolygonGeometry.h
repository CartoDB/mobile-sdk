/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MULTIPOLYGONGEOMETRY_H_
#define _CARTO_MULTIPOLYGONGEOMETRY_H_

#include "geometry/MultiGeometry.h"
#include "geometry/PolygonGeometry.h"

namespace carto {
    
    /**
     * A multipolygon container.
     */
    class MultiPolygonGeometry : public MultiGeometry {
    public:
        /**
         * Constructs a MultiPolygonGeometry from the vector of polygons.
         * @param geometries The vector of polygon geometries.
         */
        explicit MultiPolygonGeometry(const std::vector<std::shared_ptr<PolygonGeometry> >& geometries);
        virtual ~MultiPolygonGeometry();
        
        /**
         * Returns the polygon geometry at the specified index. Index must be between 0 and getGeometryCount (exclusive)
         * @param index The index of the polygon geometry.
         * @return The polygon geometry at specified index.
         * @throws std::out_of_range If the index is out of range.
         */
        std::shared_ptr<PolygonGeometry> getGeometry(int index) const;
    };
    
}

#endif
