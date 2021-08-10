/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPCLICKINFO_H_
#define _CARTO_MAPCLICKINFO_H_

#include "core/MapPos.h"
#include "ui/ClickInfo.h"

namespace carto {

    /**
     * A container class that provides information about a click performed on
     * an empty area of the map.
     */
    class MapClickInfo {
    public:
        /**
         * Constructs a MapClickInfo object from a click info and a click position.
         * @param clickInfo The click info.
         * @param clickPos The click position in the coordinate system of the base projection.
         */
        MapClickInfo(const ClickInfo& clickInfo, const MapPos& clickPos);
        virtual ~MapClickInfo();
    
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
         * @return The click position in the coordinate system of the base projection.
         */
        const MapPos& getClickPos() const;
    
    private:
        ClickInfo _clickInfo;

        MapPos _clickPos;
    };
    
}

#endif
