/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_BITMAP_H_
#define _CARTO_VT_BITMAP_H_

#include <cstdint>
#include <vector>
#include <memory>

namespace carto { namespace vt {
    constexpr static float BITMAP_SDF_SCALE = 16.0f;

    struct Bitmap final {
        const int width;
        const int height;
        const std::vector<std::uint32_t> data;

        explicit Bitmap(int width, int height, std::vector<std::uint32_t> data) : width(width), height(height), data(std::move(data)) { }
    };

    struct BitmapImage final {
        const bool sdfMode;
        const float scale;
        const std::shared_ptr<const Bitmap> bitmap;

        explicit BitmapImage(bool sdfMode, float scale, std::shared_ptr<const Bitmap> bitmap) : sdfMode(sdfMode), scale(scale), bitmap(std::move(bitmap)) { }
    };

    struct BitmapPattern final {
        const float widthScale;
        const float heightScale;
        const std::shared_ptr<const Bitmap> bitmap;

        explicit BitmapPattern(float widthScale, float heightScale, std::shared_ptr<const Bitmap> bitmap) : widthScale(widthScale), heightScale(heightScale), bitmap(std::move(bitmap)) { }
    };
} }

#endif
