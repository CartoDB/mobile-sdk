/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SKYBITMAPGENERATOR_H_
#define _CARTO_SKYBITMAPGENERATOR_H_

#include "graphics/Bitmap.h"
#include "graphics/Color.h"

namespace carto {

    /**
     * A generator for sky bitmaps.
     */
    class SkyBitmapGenerator {
    public:
        SkyBitmapGenerator(int width, int height, int gradientSize, int gradientOffset) : _width(width), _height(height), _gradientSize(gradientSize), _gradientOffset(gradientOffset) { }

        std::shared_ptr<Bitmap> generateBitmap(const Color& backgroundColor) const;
        std::shared_ptr<Bitmap> generateBitmap(const Color& backgroundColor, const Color& skyColor) const;

    private:
        static const unsigned int LIGHT_SKY_COLOR = 0xff95c4ff;
        static const unsigned int DARK_SKY_COLOR  = 0xff003880;

        const int _width;
        const int _height;
        const int _gradientSize;
        const int _gradientOffset;
    };

}

#endif
