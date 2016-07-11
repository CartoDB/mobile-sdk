/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LINE_H_
#define _CARTO_LINE_H_

#include "vectorelements/VectorElement.h"

#include <vector>

namespace carto {
    class LineDrawData;
    class LineGeometry;
    class LineStyle;
    class MapPos;
    
    /**
     * A geometric line string that can be displayed on the map.
     */
    class Line : public VectorElement {
    public:
        /**
         * Constructs a Line object from a geometry object and a style.
         * @param geometry The geometry object that defines the location of this line.
         * @param style The style that defines what this line looks like.
         */
        Line(const std::shared_ptr<LineGeometry>& geometry, const std::shared_ptr<LineStyle>& style);
        /**
         * Constructs a Line object from a vector of map positions and a style.
         * @param poses The vector of map positions that defines the location of this line.
         * @param style The style that defines what this line looks like.
         */
        Line(const std::vector<MapPos>& poses, const std::shared_ptr<LineStyle>& style);
        virtual ~Line();
    
        std::shared_ptr<LineGeometry> getGeometry() const;
        /**
         * Sets the location for this line.
         * @param geometry The new geometry object that defines the location of this line.
         */
        void setGeometry(const std::shared_ptr<LineGeometry>& geometry);

        /**
         * Returns the vertices that define this line.
         * @return The vector of map positions that defines this line.
         */
        std::vector<MapPos> getPoses() const;
        /**
         * Sets the vertices that define this line.
         * @param poses The new vector of map positions that defines this line.
         */
        void setPoses(const std::vector<MapPos>& poses);
    
        /** 
         * Returns the style of this line.
         * @return The style that defines what this line looks like.
         */
        std::shared_ptr<LineStyle> getStyle() const;
        /**
         * Sets the style for this line.
         * @param style The new style that defines what this line looks like.
         */
        void setStyle(const std::shared_ptr<LineStyle>& style);
        
        std::shared_ptr<LineDrawData> getDrawData() const;
        void setDrawData(const std::shared_ptr<LineDrawData>& drawData);

    protected:
        friend class LineRenderer;
        friend class VectorLayer;
    
    private:
        std::shared_ptr<LineDrawData> _drawData;
        
        std::shared_ptr<LineStyle> _style;
    };
    
}

#endif
