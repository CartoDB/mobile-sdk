/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_BITMAPCANVAS_H_
#define _CARTO_VT_BITMAPCANVAS_H_

#include "Color.h"
#include "Bitmap.h"

#include <cstdint>
#include <vector>
#include <memory>

namespace carto { namespace vt {
    class BitmapCanvas final {
    public:
        explicit BitmapCanvas(int width, int height);

        void setOpacity(float opacity);
        void setColor(const Color& color);

        void drawPixel(int x, int y, float a);
        void drawRectangle(float x0, float y0, float x1, float y1);
        void drawTriangle(float x0, float y0, float x1, float y1, float x2, float y2);
        void drawEllipse(float x0, float y0, float rx, float ry);

        std::shared_ptr<Bitmap> buildBitmap() const;

    private:
        const int _width;
        const int _height;
        float _opacity = 1.0f;
        std::array<std::uint8_t, 4> _color = { { 255, 255, 255, 255 } };
        std::vector<std::uint32_t> _data;
    };
} }

#endif
