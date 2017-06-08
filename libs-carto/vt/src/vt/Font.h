/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_FONT_H_
#define _CARTO_VT_FONT_H_

#include "Bitmap.h"
#include "GlyphMap.h"

#include <memory>
#include <string>
#include <cstdint>

#include <cglib/vec.h>

namespace carto { namespace vt {
    class Font {
    public:
        enum {
            SPACE_CODEPOINT   = 0xffff0000,
            CR_CODEPOINT      = 0xffff0001
        };

		using CodePoint = unsigned int;

        struct Metrics {
            float ascent;
            float descent;
            float height;
			float sdfScale;

            explicit Metrics(float ascent, float descent, float height, float sdfScale) : ascent(ascent), descent(descent), height(height), sdfScale(sdfScale) { }
        };

		struct Glyph {
			CodePoint codePoint;
			GlyphMap::Glyph baseGlyph;
			cglib::vec2<float> size;
			cglib::vec2<float> offset;
			cglib::vec2<float> advance;

			explicit Glyph(CodePoint codePoint, const GlyphMap::Glyph& baseGlyph, const cglib::vec2<float>& size, const cglib::vec2<float>& offset, const cglib::vec2<float>& advance) : codePoint(codePoint), baseGlyph(baseGlyph), size(size), offset(offset), advance(advance) { }
		};

        virtual ~Font() = default;

        virtual const Metrics& getMetrics() const = 0;
        virtual std::vector<Glyph> shapeGlyphs(const std::uint32_t* utf32Text, std::size_t size, bool rtl) const = 0;
        virtual std::shared_ptr<GlyphMap> getGlyphMap() const = 0;
    };
} }

#endif
