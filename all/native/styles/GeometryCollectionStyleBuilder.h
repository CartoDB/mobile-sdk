/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOMETRYCOLLECTIONSTYLEBUILDER_H_
#define _CARTO_GEOMETRYCOLLECTIONSTYLEBUILDER_H_

#include "styles/StyleBuilder.h"

#include <memory>

namespace carto {
    class PointStyle;
    class LineStyle;
    class PolygonStyle;
    class GeometryCollectionStyle;
    
    /**
     * A builder class for GeometryCollectionStyle.
     */
    class GeometryCollectionStyleBuilder : public StyleBuilder {
    public:
        /**
         * Constructs a GeometryCollectionStyleBuilder object with all parameters set to defaults.
         */
        GeometryCollectionStyleBuilder();
        virtual ~GeometryCollectionStyleBuilder();
        
        /**
         * Returns the point style.
         * @return The point style.
         */
        std::shared_ptr<PointStyle> getPointStyle() const;
        /**
         * Sets the style that will be used for drawing points. The default is null style.
         * @param pointStyle The new point style.
         */
        void setPointStyle(const std::shared_ptr<PointStyle>& pointStyle);
        
        /**
         * Returns the line style.
         * @return The line style.
         */
        std::shared_ptr<LineStyle> getLineStyle() const;
        /**
         * Sets the style that will be used for drawing lines. The default is null style.
         * @param lineStyle The new line style.
         */
        void setLineStyle(const std::shared_ptr<LineStyle>& lineStyle);
        
        /**
         * Returns the polygon style.
         * @return The polygon style.
         */
        std::shared_ptr<PolygonStyle> getPolygonStyle() const;
        /**
         * Sets the style that will be used for drawing polygons. The default is null style.
         * @param polygonStyle The new polygon style.
         */
        void setPolygonStyle(const std::shared_ptr<PolygonStyle>& polygonStyle);
        
        /**
         * Builds a new instance of the GeometryCollectionStyle object using previously set parameters.
         * @return A new GeometryCollectionStyle object.
         */
        std::shared_ptr<GeometryCollectionStyle> buildStyle() const;

    protected:
        std::shared_ptr<PointStyle> _pointStyle;
        std::shared_ptr<LineStyle> _lineStyle;
        std::shared_ptr<PolygonStyle> _polygonStyle;
    };
    
}

#endif
