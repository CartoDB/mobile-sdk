/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POPUPDRAWDATA_H_
#define _CARTO_POPUPDRAWDATA_H_

#include "core/ScreenPos.h"
#include "renderers/drawdatas/BillboardDrawData.h"

namespace carto {
    class Options;
    class Popup;
    class PopupStyle;
    class ViewState;
    
    class PopupDrawData : public BillboardDrawData {
    public:
        PopupDrawData(Popup& popup, const PopupStyle& style, const Projection& projection, const ProjectionSurface& projectionSurface, const std::shared_ptr<Options>& options, const ViewState& viewState);
        virtual ~PopupDrawData();

    private:
        static ScreenPos CalculateAnchorScreenPos(const Popup& popup, const ViewState& viewState, const std::shared_ptr<Options>& options, const Projection& projection);
    };
    
}

#endif
