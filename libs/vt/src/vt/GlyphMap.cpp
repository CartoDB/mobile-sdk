#include "GlyphMap.h"

#include <atomic>
#include <mutex>
#include <memory>
#include <array>
#include <map>
#include <unordered_map>

namespace carto { namespace vt {
    GlyphMap::GlyphMap(int maxWidth, int maxHeight) : _maxWidth(maxWidth), _maxHeight(maxHeight) {
        _glyphMap[0] = std::unique_ptr<Glyph>(new Glyph(0, 0, 0, 0, 0, cglib::vec2<float>(0, 0), cglib::vec2<float>(0, 0), cglib::vec2<float>(0, 0)));
    }

    const std::unique_ptr<const GlyphMap::Glyph>& GlyphMap::getGlyph(GlyphId glyphId) const {
        static std::unique_ptr<const Glyph> nullGlyph;

        std::lock_guard<std::mutex> lock(_mutex);
        
        auto it = _glyphMap.find(glyphId);
        if (it == _glyphMap.end()) {
            return nullGlyph;
        }
        return it->second;
    }

    GlyphMap::GlyphId GlyphMap::loadBitmapGlyph(const std::shared_ptr<const Bitmap>& bitmap, CodePoint codePoint) {
        if (!bitmap) {
            return 0;
        }

        cglib::vec2<float> size(static_cast<float>(bitmap->width), static_cast<float>(bitmap->height));
        cglib::vec2<float> offset(0, 0);
        cglib::vec2<float> advance(static_cast<float>(bitmap->width), 0);
        return loadBitmapGlyph(bitmap, codePoint, size, offset, advance);
    }

    GlyphMap::GlyphId GlyphMap::loadBitmapGlyph(const std::shared_ptr<const Bitmap>& bitmap, CodePoint codePoint, const cglib::vec2<float>& size, const cglib::vec2<float>& offset, const cglib::vec2<float>& advance) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!bitmap) {
            return 0;
        }

        auto it = _bitmapGlyphMap.find(bitmap);
        if (it != _bitmapGlyphMap.end()) {
            return it->second;
        }

        if (bitmap->width + 2 > _maxWidth) {
            return 0;
        }
        if (_buildState.x0 + bitmap->width + 2 > _maxWidth) {
            _buildState.y0 = _buildState.y1;
            _buildState.x0 = 0;
        }
        if (_buildState.y0 + bitmap->height + 2 > _maxHeight) {
            return 0;
        }

        _buildState.x1 = std::max(_buildState.x1, _buildState.x0 + bitmap->width + 2);
        _buildState.y1 = std::max(_buildState.y1, _buildState.y0 + bitmap->height + 2);
        if (_buildState.y1 * _maxWidth > static_cast<int>(_buildState.bitmapData.size())) {
            _buildState.bitmapData.resize((_buildState.y1 + 16) * _maxWidth);
        }

        for (int y = 0; y < bitmap->height; y++) {
            const std::uint32_t* row = &bitmap->data[y * bitmap->width];
            std::copy(row, row + bitmap->width, &_buildState.bitmapData[(_buildState.y0 + y + 1) * _maxWidth + _buildState.x0 + 1]);
        }

        GlyphId glyphId = static_cast<GlyphId>(_glyphMap.size());
        _glyphMap[glyphId] = std::unique_ptr<const Glyph>(new Glyph(codePoint, _buildState.x0 + 1, _buildState.y0 + 1, bitmap->width, bitmap->height, size, offset, advance));
        _bitmapGlyphMap[bitmap] = glyphId;

        _buildState.x0 += bitmap->width + 2;

        _bitmapPattern.reset();

        return glyphId;
    }

    std::shared_ptr<const BitmapPattern> GlyphMap::getBitmapPattern() const {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_bitmapPattern) {
            int width = 1;
            while (width < _buildState.x1) { width *= 2; }
            int height = 1;
            while (height < _buildState.y1) { height *= 2; }

            std::vector<std::uint32_t> data(width * height);
            for (int y = 0; y < _buildState.y1; y++) {
                std::copy(&_buildState.bitmapData[y * _maxWidth], &_buildState.bitmapData[y * _maxWidth] + _buildState.x1, &data[y * width]);
            }

            _bitmapPattern = std::make_shared<BitmapPattern>(1.0f, 1.0f, std::make_shared<Bitmap>(width, height, std::move(data)));
        }

        return _bitmapPattern;
    }
}}
