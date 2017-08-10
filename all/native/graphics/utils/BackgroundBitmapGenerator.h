/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BACKGROUNDBITMAPGENERATOR_H_
#define _CARTO_BACKGROUNDBITMAPGENERATOR_H_

#include "graphics/Bitmap.h"
#include "graphics/Color.h"

namespace carto {

    /**
     * A generator for background bitmaps.
     */
    class BackgroundBitmapGenerator {
    public:
        BackgroundBitmapGenerator(int blockSize, int blockCount) : _blockSize(blockSize), _blockCount(blockCount) { }

        std::shared_ptr<Bitmap> generateBitmap(const Color& backgroundColor) const;
        std::shared_ptr<Bitmap> generateBitmap(const Color& backgroundColor, const Color& dotColor) const;

    private:
        static const int DEFAULT_CONTRAST_DIFF = 40;

        const int _blockSize;
        const int _blockCount;
    };

}

#endif
