#include "SkyBitmapGenerator.h"

#include <vector>
#include <algorithm>

namespace carto {

    SkyBitmapGenerator::SkyBitmapGenerator(int width, int height) :
        _width(width),
        _height(height)
    {
    }

    SkyBitmapGenerator::~SkyBitmapGenerator() {
    }

    std::shared_ptr<Bitmap> SkyBitmapGenerator::generateBitmap(const Color& backgroundColor, const Color& skyColor) const {
        std::vector<unsigned char> data(_width * _height * 4);

        unsigned char baseValue = std::min(128, 255 - std::max(backgroundColor.getR(), std::max(backgroundColor.getG(), backgroundColor.getB())));
        Color baseColor(
            std::max(skyColor.getR(), baseValue) - baseValue,
            std::max(skyColor.getG(), baseValue) - baseValue,
            std::max(skyColor.getB(), baseValue) - baseValue,
            skyColor.getA()
        );

        for (int y = 0; y < _height; y++) {
			float v = y / (_height - 1.0f);
            float a = v * baseColor.getA() / 255.0f;
            float t = v * v;
            Color color(
                static_cast<unsigned char>(((1 - t) * baseColor.getR() + t * backgroundColor.getR()) * a),
                static_cast<unsigned char>(((1 - t) * baseColor.getG() + t * backgroundColor.getG()) * a),
                static_cast<unsigned char>(((1 - t) * baseColor.getB() + t * backgroundColor.getB()) * a),
                static_cast<unsigned char>(255 * a)
            );

            for (int x = 0; x < _width; x++) {
                data[(y * _width + x) * 4 + 0] = color.getR();
                data[(y * _width + x) * 4 + 1] = color.getG();
                data[(y * _width + x) * 4 + 2] = color.getB();
                data[(y * _width + x) * 4 + 3] = color.getA();
            }
        }

        return std::make_shared<Bitmap>(data.data(), _width, _height, ColorFormat::COLOR_FORMAT_RGBA, 4 * _width);
    }

}
