#include "SkyBitmapGenerator.h"

#include <vector>
#include <algorithm>

namespace carto {

    std::shared_ptr<Bitmap> SkyBitmapGenerator::generateBitmap(const Color& backgroundColor) const {
        unsigned char maxComponent = std::max(backgroundColor.getR(), std::max(backgroundColor.getG(), backgroundColor.getB()));
        return generateBitmap(backgroundColor, maxComponent >= 128 ? Color(LIGHT_SKY_COLOR) : Color(DARK_SKY_COLOR));
    }

    std::shared_ptr<Bitmap> SkyBitmapGenerator::generateBitmap(const Color& backgroundColor, const Color& skyColor) const {
        std::vector<unsigned char> data(_width * _height * 4);

        for (int y = 0; y < _height; y++) {
            float s = std::max(0.0f, std::min(1.0f, static_cast<float>(y - _height / 2 + _gradientSize / 2 - _gradientOffset) / _gradientSize));
            Color color(static_cast<unsigned char>((1 - s) * skyColor.getR() + s * backgroundColor.getR()),
                        static_cast<unsigned char>((1 - s) * skyColor.getG() + s * backgroundColor.getG()),
                        static_cast<unsigned char>((1 - s) * skyColor.getB() + s * backgroundColor.getB()),
                        255);

            unsigned char components[4] = { color.getR(), color.getG(), color.getB(), color.getA() };
            for (int i = 0; i < _width * 4; i++) {
                data[y * _width * 4 + i] = components[i % 4];
            }
        }

        return std::make_shared<Bitmap>(data.data(), _width, _height, ColorFormat::COLOR_FORMAT_RGBA, 4 * _width);
    }

}
