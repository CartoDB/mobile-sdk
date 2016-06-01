/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MARKERDRAWDATA_H_
#define _CARTO_MARKERDRAWDATA_H_

#include "renderers/drawdatas/BillboardDrawData.h"

namespace carto {
    class Marker;
    class MarkerStyle;
    
    class MarkerDrawData: public BillboardDrawData {
    public:
        MarkerDrawData(const Marker& marker, const MarkerStyle& style, const Projection& projection);
        virtual ~MarkerDrawData();
    };
    
}

#endif
