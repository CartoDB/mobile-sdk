/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORELEMENTCLICKINFO_H_
#define _CARTO_VECTORELEMENTCLICKINFO_H_

#include "core/MapPos.h"
#include "ui/ClickType.h"
#include "vectorelements/VectorElement.h"

#include <memory>

namespace carto {
    class Layer;
    
    /**
     * A container class that provides information about a click performed on
     * a vector element.
     */
    class VectorElementClickInfo {
    public:
        /**
         * Constructs a VectorElementClickInfo object from a click position and a vector element.
         * @param clickType The click type (SINGLE, DUAL, etc)
         * @param clickPos The click position in the coordinate system of the base projection.
         * @param elementClickPos The click position in the coordinate system of the base projection that corresponds to element point.
         * @param vectorElement The vector element on which the click was performed.
         * @param layer The layer of the vector element on which the click was performed.
         */
        VectorElementClickInfo(ClickType::ClickType clickType, const MapPos& clickPos, const MapPos& elementClickPos, const std::shared_ptr<VectorElement>& vectorElement, const std::shared_ptr<Layer>& layer);
        virtual ~VectorElementClickInfo();
    
        /**
         * Returns the click type.
         * @return The type of the click performed.
         */
        ClickType::ClickType getClickType() const;

        /**
         * Returns the click position.
         * @return The click position in the coordinate system of the base projection.
         */
        const MapPos& getClickPos() const;
        
        /**
         * Returns the position on the clicked element, that is close to the click position.
         * For points it will always be the center position, for lines it will be the closest point
         * on the line, for billboards it will be the anchor point and for polygons it's equal to
         * getClickPos().
         * @return The element click position in the coordinate system of the base projection.
         */
        const MapPos& getElementClickPos() const;
    
        /**
         * Returns the clicked vector element.
         * @return The vector element on which the click was performed.
         */
        std::shared_ptr<VectorElement> getVectorElement() const;

        /**
         * Returns the layer of the clicked vector element.
         * @return The layer of the vector element on which the click was performed.
         */
        std::shared_ptr<Layer> getLayer() const;
    
    private:
        ClickType::ClickType _clickType;
        MapPos _clickPos;
        MapPos _elementClickPos;
    
        std::shared_ptr<VectorElement> _vectorElement;
        std::shared_ptr<Layer> _layer;
    };
    
}

#endif
