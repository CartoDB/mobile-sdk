#include "BackgroundBitmapGenerator.h"

#include <vector>
#include <algorithm>

namespace carto {

    std::shared_ptr<Bitmap> BackgroundBitmapGenerator::generateBitmap(const Color& backgroundColor) const {
        unsigned char maxComponent = std::max(backgroundColor.getR(), std::max(backgroundColor.getG(), backgroundColor.getB()));
        unsigned char dotComponent = (maxComponent >= 128 ? maxComponent - DEFAULT_CONTRAST_DIFF : maxComponent + DEFAULT_CONTRAST_DIFF);
        return generateBitmap(backgroundColor, Color(dotComponent, dotComponent, dotComponent, 255));
    }

    std::shared_ptr<Bitmap> BackgroundBitmapGenerator::generateBitmap(const Color& backgroundColor, const Color& dotColor) const {
        std::vector<unsigned char> data(_width * _height * 4);

        unsigned char bgComponents[4] = { backgroundColor.getR(), backgroundColor.getG(), backgroundColor.getB(), backgroundColor.getA() };
        for (std::size_t i = 0; i < data.size(); i++) {
            data[i] = bgComponents[i % 4];
        }

        for (unsigned int i = 0; i < _width; i += 2) {
            data[i * 4 + 0] = dotColor.getR();
            data[i * 4 + 1] = dotColor.getG();
            data[i * 4 + 2] = dotColor.getB();
            data[i * 4 + 3] = dotColor.getA();
        }

        for (unsigned int i = 0; i < _height; i += 2) {
            data[i * _width * 4 + 0] = dotColor.getR();
            data[i * _width * 4 + 1] = dotColor.getG();
            data[i * _width * 4 + 2] = dotColor.getB();
            data[i * _width * 4 + 3] = dotColor.getA();
        }

        return std::make_shared<Bitmap>(data.data(), _width, _height, ColorFormat::COLOR_FORMAT_RGBA, 4 * _width);
    }

}
