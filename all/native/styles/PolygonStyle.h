/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POLYGONSTYLE_H_
#define _CARTO_POLYGONSTYLE_H_

#include "styles/Style.h"

#include <memory>

namespace carto {
    class Bitmap;
    class LineStyle;
    
    /**
     * A style for polygons. Contains attributes for configuring how the polygon is drawn on the screen.
     */
    class PolygonStyle : public Style {
    public:
        /**
         * Constructs a PolygonStyle object from various parameters. Instantiating the object directly is
         * not recommended, PolygonStyleBuilder should be used instead.
         * @param color The color for the polygon.
         * @param bitmap The bitmap for the polygon.
         * @param lineStyle The line style for the edges of the polygon. May be null.
         */
        PolygonStyle(const Color& color, const std::shared_ptr<Bitmap>& bitmap,
                const std::shared_ptr<LineStyle>& lineStyle);
        virtual ~PolygonStyle();
    
        /**
         * Returns the bitmap of the polygon.
         * @return The bitmap of the polygon.
         */
        std::shared_ptr<Bitmap> getBitmap() const;
    
        /**
         * Returns the style of the edges of the polygon.
         * @return The style of the edges of the polygon. May be null.
         */
        std::shared_ptr<LineStyle> getLineStyle() const;
    
    protected:
        std::shared_ptr<Bitmap> _bitmap;
    
        std::shared_ptr<LineStyle> _lineStyle;
    };
    
}

#endif
