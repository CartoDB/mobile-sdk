/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LABELDRAWDATA_H_
#define _CARTO_LABELDRAWDATA_H_

#include "renderers/drawdatas/BillboardDrawData.h"

namespace carto {
    class Label;
    class LabelStyle;
    class ViewState;
    
    class LabelDrawData: public BillboardDrawData {
    public:
        LabelDrawData(const Label& label, const LabelStyle& style,
                      const Projection& projection, const ViewState& viewState);
        virtual ~LabelDrawData();
    };
    
}

#endif
