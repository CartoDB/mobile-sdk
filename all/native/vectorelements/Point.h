/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POINT_H_
#define _CARTO_POINT_H_

#include "vectorelements/VectorElement.h"

namespace carto {
    class PointDrawData;
    class PointGeometry;
    class PointStyle;
    class MapPos;
    
    /**
     * A geometric point that can be displayed on the map.
     */
    class Point : public VectorElement {
    public:
        /**
         * Constructs a Point object from a geometry object and a style.
         * @param geometry The geometry object that defines the location of this point.
         * @param style The style that defines what this point looks like.
         */
        Point(const std::shared_ptr<PointGeometry>& geometry, const std::shared_ptr<PointStyle>& style);
        /**
         * Constructs a Point object from a map position and a style.
         * @param pos The map position that defines the location of this point.
         * @param style The style that defines what this point looks like.
         */
        Point(const MapPos& pos, const std::shared_ptr<PointStyle>& style);
        virtual ~Point();
    
        std::shared_ptr<PointGeometry> getGeometry() const;
        /**
         * Sets the location for this point.
         * @param geometry The new geometry object that defines the location of this point.
         */
        void setGeometry(const std::shared_ptr<PointGeometry>& geometry);

        /**
         * Returns the location of this point.
         * @return The map position that defines the location of this point.
         */
        MapPos getPos() const;
        /**
         * Sets the location of this point.
         * @param pos The new map position that defines the location of this point.
         */
        void setPos(const MapPos& pos);
    
        /**
         * Returns the style of this point.
         * @return The style that defines what this point looks like.
         */
        std::shared_ptr<PointStyle> getStyle() const;
        /**
         * Sets a style for this point.
         * @param style The new style that defines what this point looks like.
         */
        void setStyle(const std::shared_ptr<PointStyle>& style);
    
        std::shared_ptr<PointDrawData> getDrawData() const;
        void setDrawData(const std::shared_ptr<PointDrawData>& drawData);

    protected:
        friend class PointRenderer;
        friend class VectorLayer;
        
    private:
        std::shared_ptr<PointDrawData> _drawData;
        
        std::shared_ptr<PointStyle> _style;
    };
    
}

#endif
