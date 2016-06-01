/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_WKBGEOMETRYENUMS_H_
#define _CARTO_WKBGEOMETRYENUMS_H_

namespace carto {
    
    enum wkbGeometryType {
        wkbPoint = 1,
        wkbLineString = 2,
        wkbPolygon = 3,
        wkbMultiPoint = 4,
        wkbMultiLineString = 5,
        wkbMultiPolygon = 6,
        wkbGeometryCollection = 7,

        wkbZMask = 0x1000,
        wkbMMask = 0x2000
    };

    enum wkbBigEndian {
        wkbXDR = 0,             // Big Endian
        wkbNDR = 1           // Little Endian
    };

}

#endif
