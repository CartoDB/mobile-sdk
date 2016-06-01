/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POPUPDRAWDATA_H_
#define _CARTO_POPUPDRAWDATA_H_

#include "renderers/drawdatas/BillboardDrawData.h"

namespace carto {
    class Options;
    class Popup;
    class PopupStyle;
    class ViewState;
    
    class PopupDrawData: public BillboardDrawData {
    public:
        PopupDrawData(Popup& popup, const PopupStyle& style, const Projection& projection, const Options& options, const ViewState& viewState);
        virtual ~PopupDrawData();
    };
    
}

#endif
