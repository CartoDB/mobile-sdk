/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MULTILINEGEOMETRY_H_
#define _CARTO_MULTILINEGEOMETRY_H_

#include "geometry/MultiGeometry.h"
#include "geometry/LineGeometry.h"

namespace carto {
    
    /**
     * A multiline container.
     */
    class MultiLineGeometry : public MultiGeometry {
    public:
        /**
         * Constructs a MultiLineGeometry object from the vector of lines.
         * @param geometries The vector of line geometries.
         */
        explicit MultiLineGeometry(const std::vector<std::shared_ptr<LineGeometry> >& geometries);
        virtual ~MultiLineGeometry();
        
        /**
         * Returns the line geometry at the specified index. Index must be between 0 and getGeometryCount (exclusive)
         * @param index The index of the line geometry.
         * @return The line geometry at specified index.
         * @throws std::out_of_range If the index is out of range.
         */
        std::shared_ptr<LineGeometry> getGeometry(int index) const;
    };
    
}

#endif
