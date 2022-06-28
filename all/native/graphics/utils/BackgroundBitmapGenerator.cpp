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
        int size = _blockSize * _blockCount;
        std::vector<unsigned char> data(size * size * 4, 255);

        for (int i = 0; i < size * size; i++) {
            data[i * 4 + 0] = backgroundColor.getR();
            data[i * 4 + 1] = backgroundColor.getG();
            data[i * 4 + 2] = backgroundColor.getB();
        }

        for (int i = 0; i < _blockCount; i++) {
            for (int j = 0; j + 1 < size; j += 2) {
                int i0 = i * size * _blockSize + j;
                data[i0 * 4 + 0] = dotColor.getR();
                data[i0 * 4 + 1] = dotColor.getG();
                data[i0 * 4 + 2] = dotColor.getB();

                int i1 = i * _blockSize + j * size;
                data[i1 * 4 + 0] = dotColor.getR();
                data[i1 * 4 + 1] = dotColor.getG();
                data[i1 * 4 + 2] = dotColor.getB();
            }
        }

        return std::make_shared<Bitmap>(data.data(), size, size, ColorFormat::COLOR_FORMAT_RGBA, 4 * size);
    }

    const int BackgroundBitmapGenerator::DEFAULT_CONTRAST_DIFF = 40;

}
