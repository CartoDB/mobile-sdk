/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOUIBUILDER_H_
#define _CARTO_CARTOUIBUILDER_H_

#include "core/Variant.h"

#include <memory>

namespace carto {
    class Layer;

    class CartoUIBuilder {
    public:
        virtual ~CartoUIBuilder() { }

        virtual void createLegend(const std::shared_ptr<Layer>& layer, const Variant& legendInfo) { }
        virtual void createInfoWindow(const std::shared_ptr<Layer>& layer, const Variant& infoWindow) { }
    };

}

#endif
