/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_STYLEBUILDER_H_
#define _CARTO_STYLEBUILDER_H_

#include "graphics/Color.h"

#include <mutex>

namespace carto {

    /**
     * A base class for other StyleBuilder subclasses.
     */
    class StyleBuilder {
    public:
        virtual ~StyleBuilder();
        
        /**
         * Returns the color of the vector element.
         * @return The color of the vector element.
         */
        Color getColor() const;
        /**
         * Sets the color for the vector element. Coloring works by multiplying the bitmap
         * of the vector element with the specified color. If the color is set to white, the resulting bitmap
         * will look exactly like the original. The default is white.
         * @param color The new color for the vector element.
         */
        void setColor(const Color& color);
    
    protected:
        StyleBuilder();
        
        Color _color;
        
        mutable std::mutex _mutex;
    };
    
}

#endif
