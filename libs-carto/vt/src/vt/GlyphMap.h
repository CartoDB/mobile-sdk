/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_GLYPHMAP_H_
#define _CARTO_VT_GLYPHMAP_H_

#include "Bitmap.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <mutex>

#include <cglib/vec.h>

namespace carto { namespace vt {
    class GlyphMap final {
    public:
        using GlyphId = unsigned int;
        
        struct Glyph {
			bool sdf;
            int x;
            int y;
            int width;
            int height;
			cglib::vec2<float> origin;

            explicit Glyph(bool sdf, int x, int y, int width, int height, const cglib::vec2<float>& origin) : sdf(sdf), x(x), y(y), width(width), height(height), origin(origin) { }
        };

        explicit GlyphMap(int maxWidth, int maxHeight);

        const Glyph* getGlyph(GlyphId code) const;
		GlyphId loadBitmapGlyph(const std::shared_ptr<const Bitmap>& bitmap, bool sdf);
		GlyphId loadBitmapGlyph(const std::shared_ptr<const Bitmap>& bitmap, bool sdf, const cglib::vec2<float>& origin);
        std::shared_ptr<const BitmapPattern> getBitmapPattern() const;

    private:
        struct BuildState {
            int x0 = 0;
            int x1 = 0;
            int y0 = 0;
            int y1 = 0;
            std::vector<std::uint32_t> bitmapData;

            BuildState() = default;
        };

        const int _maxWidth;
        const int _maxHeight;
        std::unordered_map<std::shared_ptr<const Bitmap>, GlyphId> _bitmapGlyphMap;
        std::unordered_map<GlyphId, std::unique_ptr<const Glyph>> _glyphMap;
        mutable std::shared_ptr<BitmapPattern> _bitmapPattern;
        BuildState _buildState;
        mutable std::mutex _mutex;
    };
} }

#endif
