/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POLYGONSTYLEBUILDER_H_
#define _CARTO_POLYGONSTYLEBUILDER_H_

#include "styles/StyleBuilder.h"

#include <memory>

namespace carto {
    class Bitmap;
    class LineStyle;
    class PolygonStyle;
    
    /**
     * A builder class for PolygonStyle.
     */
    class PolygonStyleBuilder : public StyleBuilder {
    public:
        /**
         * Constructs a PolygonStyleBuilder object with all parameters set to defaults.
         */
        PolygonStyleBuilder();
        virtual ~PolygonStyleBuilder();
    
        /**
         * Returns the line style of the edges of the polygon.
         * @return The line style of the edges of the polygon. May be null.
         */
        std::shared_ptr<LineStyle> getLineStyle() const;
        /**
         * Sets the line style that will be used to draw the edges of the polygon. If null is passed
         * no edges will be drawn. The default is null.
         * @param lineStyle The new style for the edges of the polygon.
         */
        void setLineStyle(const std::shared_ptr<LineStyle>& lineStyle);
    
        /**
         * Builds a new instance of the PolygonStyle object using previously set parameters.
         * @return A new PolygonStyle object.
         */
        std::shared_ptr<PolygonStyle> buildStyle() const;
    
    protected:
        static std::shared_ptr<Bitmap> GetDefaultBitmap();
    
        static std::shared_ptr<Bitmap> _DefaultBitmap;
        static std::mutex _DefaultBitmapMutex;
        
        std::shared_ptr<Bitmap> _bitmap;
    
        std::shared_ptr<LineStyle> _lineStyle;
    };
    
}

#endif
