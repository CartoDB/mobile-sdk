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
        SkyBitmapGenerator(int width, int height);
        virtual ~SkyBitmapGenerator();

        std::shared_ptr<Bitmap> generateBitmap(const Color& groundColor, const Color& skyColor) const;

    private:
        const int _width;
        const int _height;
    };

}

#endif
