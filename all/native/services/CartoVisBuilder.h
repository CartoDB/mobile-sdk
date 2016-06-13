/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOVISBUILDER_H_
#define _CARTO_CARTOVISBUILDER_H_

#include "core/MapPos.h"
#include "core/MapBounds.h"
#include "core/Variant.h"

#include <memory>

namespace carto {
    class Layer;

    class CartoVisBuilder {
    public:
        virtual ~CartoVisBuilder() { }

        virtual void setCenter(const MapPos& mapPos) { }
        virtual void setZoom(float zoom) { }
        virtual void setBounds(const MapBounds& mapBounds) { }
        virtual void setDescription(const Variant& descriptionInfo) { }

        virtual void addLayer(const std::shared_ptr<Layer>& layer, const Variant& attributes) { }
    };

}

#endif
