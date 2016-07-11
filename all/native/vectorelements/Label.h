/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LABEL_H_
#define _CARTO_LABEL_H_

#include "vectorelements/Billboard.h"

namespace carto {
    class Bitmap;
    class LabelDrawData;
    class LabelStyle;
    
    /**
     * An abstract billboard element with a dynamically drawn bitmap that can be displayed on the map.
     */
    class Label : public Billboard {
    public:
        /**
         * Constructs an abstract Label object with the specified style and attaches it to a billboard element.
         * @param baseBillboard The billboard this label will be attached to.
         * @param style The style that defines what this label looks like.
         */
        Label(const std::shared_ptr<Billboard>& baseBillboard, const std::shared_ptr<LabelStyle>& style);
        /**
         * Constructs an abstract Label object from a geometry object and a style.
         * @param geometry The geometry object that defines the location of this label.
         * @param style The style that defines what this label looks like.
         */
        Label(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<LabelStyle>& style);
        /**
         * Constructs an abstract Label object from a map position and a style.
         * @param pos The map position that defines the location of this label.
         * @param style The style that defines what this label looks like.
         */
        Label(const MapPos& pos, const std::shared_ptr<LabelStyle>& style);
        virtual ~Label();
    
        /**
         * Draws a custom bitmap for this label that will be used for drawing the label on the map.
         * The method is called each time the label gets reloaded internally.
         * @param dpToPX The value used for converting display independent pixels (dp) to pixels (px).
         * @return The custom label bitmap.
         */
        virtual std::shared_ptr<Bitmap> drawBitmap(float dpToPX) const = 0;
    
        /**
         * Returns the style of this label.
         * @return The style that defines what this label looks like.
         */
        std::shared_ptr<LabelStyle> getStyle() const;
        /**
         * Sets the style for this label.
         * @param style The new style that defines what this marker looks like.
         */
        void setStyle(const std::shared_ptr<LabelStyle>& style);
        
    private:
        std::shared_ptr<LabelStyle> _style;
    };
    
}

#endif
