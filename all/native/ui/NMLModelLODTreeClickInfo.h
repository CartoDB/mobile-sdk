/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELLODTREECLICKINFO_H_
#define _CARTO_NMLMODELLODTREECLICKINFO_H_

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "core/MapPos.h"
#include "ui/ClickInfo.h"

#include <memory>
#include <map>
#include <string>

namespace carto {
    class Layer;
    
    /**
     * A container class that provides information about a click performed on
     * a NMLModelLODTree element.
     */
    class NMLModelLODTreeClickInfo {
    public:
        /**
         * Constructs a NMLModelLODTreeClickInfo object from a click info and a element data.
         * @param clickInfo The click info
         * @param clickPos The click position in the coordinate system of the data source.
         * @param elementClickPos The click position in the coordinate system of the data source that corresponds to element point.
         * @param metaData The element meta data at the click position.
         * @param layer The layer of the vector element on which the click was performed.
         */
        NMLModelLODTreeClickInfo(const ClickInfo& clickInfo, const MapPos& clickPos, const MapPos& elementClickPos, const std::map<std::string, std::string>& metaData, const std::shared_ptr<Layer>& layer);
        virtual ~NMLModelLODTreeClickInfo();
    
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
         * Returns the position on the clicked element, that is close to the click position.
         * @return The element click position in the coordinate system of the data source.
         */
        const MapPos& getElementClickPos() const;
    
        /**
         * Returns the element meta data at the click point.
         * @return The element meta data at the click point.
         */
        const std::map<std::string, std::string>& getMetaData() const;

        /**
         * Returns the layer of the clicked vector element.
         * @return The layer of the vector element on which the click was performed.
         */
        std::shared_ptr<Layer> getLayer() const;
    
    private:
        ClickInfo _clickInfo;
        MapPos _clickPos;
        MapPos _elementClickPos;
    
        std::map<std::string, std::string> _metaData;
        std::shared_ptr<Layer> _layer;
    };
    
}

#endif

#endif
