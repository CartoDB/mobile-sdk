/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MULTIPOINTGEOMETRY_H_
#define _CARTO_MULTIPOINTGEOMETRY_H_

#include "geometry/MultiGeometry.h"
#include "geometry/PointGeometry.h"

namespace carto {

    /**
     * A multipoint container.
     */
    class MultiPointGeometry : public MultiGeometry {
    public:
        /**
         * Constructs a MultiPointGeometry from the vector of points.
         * @geometries The vector of point geometries.
         */
        explicit MultiPointGeometry(const std::vector<std::shared_ptr<PointGeometry> >& geometries);
        virtual ~MultiPointGeometry();

        /**
         * Returns the point geometry at the specified index. Index must be between 0 and getGeometryCount (exclusive)
         * @param index The index of the point geometry.
         * @return The point geometry at specified index.
         * @throws std::out_of_range If the index is out of range.
         */
        std::shared_ptr<PointGeometry> getGeometry(int index) const;
    };

}

#endif
