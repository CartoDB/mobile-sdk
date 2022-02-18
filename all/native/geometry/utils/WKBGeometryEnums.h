/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_WKBGEOMETRYENUMS_H_
#define _CARTO_WKBGEOMETRYENUMS_H_

namespace carto {
    
    enum WKBGeometryType {
        WKB_POINT = 1,
        WKB_LINESTRING = 2,
        WKB_POLYGON = 3,
        WKB_MULTIPOINT = 4,
        WKB_MULTILINESTRING = 5,
        WKB_MULTIPOLYGON = 6,
        WKB_GEOMETRYCOLLECTION = 7,

        WKB_ZMASK = 0x1000,
        WKB_MMASK = 0x2000
    };

    enum WKBBigEndian {
        WKB_XDR = 0,             // Big Endian
        WKB_NDR = 1           // Little Endian
    };

}

#endif
