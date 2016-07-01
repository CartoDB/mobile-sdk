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

    /**
     * A callback interface for handling VisJSON directives.
     * The callback can be used to change the default VisJSON behaviour
     * by ignoring or modifying configured layers or map center position,
     * for example.
     */
    class CartoVisBuilder {
    public:
        virtual ~CartoVisBuilder() { }

        /**
         * Called when the map should be centered at given position.
         * @param mapPos The center position in WGS84 coordinates (longitude-latitude).
         */
        virtual void setCenter(const MapPos& mapPos) { }
        /**
         * Called when the map should be zoomed to the specified level.
         * @param zoom The zoom level.
         */
        virtual void setZoom(float zoom) { }
        /**
         * Called when the map should use the specified bounds.
         * @param mapBounds The map bounds in WGS84 coordinates.
         */
        virtual void setBounds(const MapBounds& mapBounds) { }
        /**
         * Called when the map should be augmented with decription info.
         * The description info does not have a formal structure and is
         * basically a JSON value encoded as a Variant.
         * @param descriptionInfo The description variant.
         */
        virtual void setDescription(const Variant& descriptionInfo) { }

        /**
         * Called when a layer should be added to the map.
         * @param layer The layer to be added. 
         * @param attributes The additional layer attributes. For example, info window and layer legend are part of the attributes.
         */
        virtual void addLayer(const std::shared_ptr<Layer>& layer, const Variant& attributes) { }
    };

}

#endif
