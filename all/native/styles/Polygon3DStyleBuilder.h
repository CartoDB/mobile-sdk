/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POLYGON3DSTYLEBUILDER_H_
#define _CARTO_POLYGON3DSTYLEBUILDER_H_

#include "styles/StyleBuilder.h"

#include <memory>

namespace carto {
    class Polygon3DStyle;
    
    /**
     * A builder class for Polygon3DStyle.
     */
    class Polygon3DStyleBuilder : public StyleBuilder {
    public:
        /**
         * Constructs a Polygon3DStyleBuilder object with all parameters set to defaults.
         */
        Polygon3DStyleBuilder();
        virtual ~Polygon3DStyleBuilder();

        /**
         * Returns the color of the 3d polygon sides.
         * @return The color of the 3d polygon sides.
         */
        Color getSideColor() const;
        /**
         * Sets the the color of the 3d polygon sides. The side color is also affected by lighting settings,
         * so that sides with different orientation are distinguishable.
         * @param sideColor The new color for the 3d polygon sides.
         */
        void setSideColor(const Color& sideColor);
    
        /**
         * Builds a new instance of the Polygon3DStyle object using previously set parameters.
         * @return A new Polygon3DStyle object.
         */
        std::shared_ptr<Polygon3DStyle> buildStyle() const;

    private:
        Color _sideColor;
        bool _sideColorDefined;
    };
    
}

#endif
