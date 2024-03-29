/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_UTFGRIDCLICKINFO_H_
#define _CARTO_UTFGRIDCLICKINFO_H_

#include "core/MapPos.h"
#include "core/Variant.h"
#include "ui/ClickInfo.h"

#include <memory>
#include <string>
#include <map>

namespace carto {
    class Layer;
    
    /**
     * A container class that provides information about a click performed on
     * an UTF grid.
     */
    class UTFGridClickInfo {
    public:
        /**
         * Constructs an UTFGridClickInfo object from a click info, click position and grid element metadata.
         * @param clickInfo The click info
         * @param clickPos The click position in the coordinate system of the data source.
         * @param elementInfo The info tag of the element that was clicked.
         * @param layer The layer on which the click was performed.
         */
        UTFGridClickInfo(const ClickInfo& clickInfo, const MapPos& clickPos, const Variant& elementInfo, const std::shared_ptr<Layer>& layer);
        virtual ~UTFGridClickInfo();
    
        /**
         * Returns the click type.
         * @return The type of the click performed.
         */
        ClickType::ClickType getClickType() const;

        /**
         * Returns the click info.
         * @return The attributes of the click.
         */
        const ClickInfo& getClickInfo() const;

        /**
         * Returns the click position.
         * @return The click position in the coordinate system of the data source.
         */
        const MapPos& getClickPos() const;
        
        /**
         * Returns the info tag of the clicked element.
         * @return The info tag of the clicked element.
         */
        const Variant& getElementInfo() const;

        /**
         * Returns the clicked layer.
         * @return The clicked layer.
         */
        std::shared_ptr<Layer> getLayer() const;
    
    private:
        ClickInfo _clickInfo;
        MapPos _clickPos;

        Variant _elementInfo;
        std::shared_ptr<Layer> _layer;
    };
    
}

#endif
