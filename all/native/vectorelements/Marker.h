/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MARKER_H_
#define _CARTO_MARKER_H_

#include "vectorelements/Billboard.h"

namespace carto {
    class MarkerDrawData;
    class MarkerStyle;
    
    /**
     * A billboard element with a static bitmap that can be displayed on the map.
     */
    class Marker : public Billboard {
    public:
        /**
         * Constructs a Marker object with the specified style and attaches it to a billboard element.
         * @param baseBillboard The billboard this billboard will be attached to.
         * @param style The style that defines what this marker looks like.
         */
        Marker(const std::shared_ptr<Billboard>& baseBillboard, const std::shared_ptr<MarkerStyle>& style);
        /**
         * Constructs a Marker object from a geometry object and a style.
         * @param geometry The geometry object that defines the location of this marker.
         * @param style The style that defines what this marker looks like.
         */
        Marker(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<MarkerStyle>& style);
        /**
         * Constructs a Marker object from a map position and a style.
         * @param pos The map position that defines the location of this marker.
         * @param style The style that defines what this marker looks like.
         */
        Marker(const MapPos& pos, const std::shared_ptr<MarkerStyle>& style);
        virtual ~Marker();
        
        /**
         * Returns the style of this marker.
         * @return The style that defines what this marker looks like.
         */
        virtual std::shared_ptr<MarkerStyle> getStyle() const;
        /**
         * Sets the style for this marker.
         * @param style The new style that defines what this marker looks like.
         */
        void setStyle(const std::shared_ptr<MarkerStyle>& style);
        
    private:
        std::shared_ptr<MarkerStyle> _style;
    };
    
}

#endif
