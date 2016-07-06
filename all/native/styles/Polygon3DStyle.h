/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POLYGON3DSTYLE_H_
#define _CARTO_POLYGON3DSTYLE_H_

#include "styles/Style.h"

#include <memory>

namespace carto {

    /**
     * A style for 3d polygons. Contains attributes for configuring how the 3d polygon is drawn on the screen.
     */
    class Polygon3DStyle : public Style {
    public:
        /**
         * Constructs a Polygon3DStyle object from various parameters. Instantiating the object directly is
         * not recommended, Polygon3DStyleBuilder should be used instead.
         * @param color The color for the 3d polygon.
         * @param sideColor The color for sides of the 3d polygon.
         */
        Polygon3DStyle(const Color& color, const Color& sideColor);
        virtual ~Polygon3DStyle();

        /**
         * Returns the color for sides of the 3d polygon.
         * @return The color for sides of the 3d polygon.
         */
        const Color& getSideColor() const;

    private:
        Color _sideColor;
    };
    
}

#endif
