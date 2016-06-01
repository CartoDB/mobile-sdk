#include "StrokeMap.h"
#include "BitmapManager.h"

namespace carto { namespace vt {
    StrokeMap::StrokeMap(int width) :
        _width(width), _height(0), _strokeMap(), _bitmapStrokeMap(), _bitmapPattern(), _mutex()
    {
        int height = 1;
        std::vector<std::uint32_t> data(width * 1, static_cast<std::uint32_t>(-1));

        _strokeMap[0] = std::unique_ptr<Stroke>(new Stroke(65536.0f, 0, height));
        _bitmapPattern = std::make_shared<BitmapPattern>(1.0f, 1.0f, std::make_shared<Bitmap>(width, 1, std::move(data)));
        _height = height;
    }

    int StrokeMap::getWidth() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _bitmapPattern->bitmap->width;
    }

    const std::unique_ptr<const StrokeMap::Stroke>& StrokeMap::getStroke(StrokeId strokeId) const {
        static std::unique_ptr<const Stroke> nullStroke;

        std::lock_guard<std::mutex> lock(_mutex);
        
        auto it = _strokeMap.find(strokeId);
        if (it == _strokeMap.end()) {
            return nullStroke;
        }
        return it->second;
    }
    
    StrokeMap::StrokeId StrokeMap::loadBitmapPattern(const std::shared_ptr<const BitmapPattern>& bitmapPattern) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (!bitmapPattern) {
            return 0;
        }
        
        auto it = _bitmapStrokeMap.find(bitmapPattern);
        if (it != _bitmapStrokeMap.end()) {
            return it->second;
        }

        std::vector<std::uint32_t> patternData(bitmapPattern->bitmap->data);
        int patternWidth = bitmapPattern->bitmap->width;
        int patternHeight = bitmapPattern->bitmap->height;
        while (patternWidth + bitmapPattern->bitmap->width <= _width) {
            int newPatternWidth = patternWidth + bitmapPattern->bitmap->width;
            std::vector<std::uint32_t> newPatternData(newPatternWidth * patternHeight);
            for (int y = 0; y < patternHeight; y++) {
                std::copy(patternData.begin() + y * patternWidth, patternData.begin() + y * patternWidth + patternWidth, newPatternData.begin() + y * newPatternWidth);
                std::copy(patternData.begin() + y * patternWidth, patternData.begin() + y * patternWidth + bitmapPattern->bitmap->width, newPatternData.begin() + y * newPatternWidth + patternWidth);
            }
            std::swap(patternData, newPatternData);
            std::swap(patternWidth, newPatternWidth);
        }

        std::shared_ptr<const Bitmap> sourceBitmap = _bitmapPattern->bitmap;
        std::shared_ptr<const Bitmap> scaledBitmap = BitmapManager::scale(std::make_shared<Bitmap>(patternWidth, patternHeight, std::move(patternData)), _width, patternHeight);

        int height = _height + scaledBitmap->height;
        int pow2Height = 1;
        while (pow2Height < height) { pow2Height *= 2; }
        std::vector<std::uint32_t> data(_width * pow2Height);
        std::copy(sourceBitmap->data.begin(), sourceBitmap->data.begin() + _width * _height, data.begin());
        std::copy(scaledBitmap->data.begin(), scaledBitmap->data.end(), data.begin() + _width * _height);

        StrokeId strokeId = static_cast<StrokeId>(_strokeMap.size());
        _strokeMap[strokeId] = std::unique_ptr<Stroke>(new Stroke(bitmapPattern->widthScale * patternWidth, _height, height));
        _bitmapStrokeMap[bitmapPattern] = strokeId;
        _bitmapPattern = std::make_shared<BitmapPattern>(1.0f, 1.0f, std::make_shared<Bitmap>(_width, pow2Height, std::move(data)));
        _height = height;
        return strokeId;
    }
    
    std::shared_ptr<const BitmapPattern> StrokeMap::getBitmapPattern() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _bitmapPattern;
    }
} }
