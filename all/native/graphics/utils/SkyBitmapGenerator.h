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
        SkyBitmapGenerator(int width, int height, int gradientSize) : _width(width), _height(height), _gradientSize(gradientSize) { }

        std::shared_ptr<Bitmap> generateBitmap(const Color& backgroundColor) const;
        std::shared_ptr<Bitmap> generateBitmap(const Color& backgroundColor, const Color& skyColor) const;

    private:
        static const int DEFAULT_CONTRAST_DIFF = 50;

        const int _width;
        const int _height;
        const int _gradientSize;
    };

}

#endif
