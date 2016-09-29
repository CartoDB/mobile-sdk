/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_TEXTFORMATTER_H_
#define _CARTO_VT_TEXTFORMATTER_H_

#include "Font.h"

#include <memory>
#include <string>
#include <vector>

#include <cglib/vec.h>
#include <cglib/bbox.h>

namespace carto { namespace vt {
    class TextFormatter final {
    public:
        struct Options {
            cglib::vec2<float> alignment;
            cglib::vec2<float> offset;
            bool wrapBefore;
            float wrapWidth;
            float characterSpacing;
            float lineSpacing;

            explicit Options(const cglib::vec2<float>& alignment, const cglib::vec2<float>& offset, bool wrapBefore, float wrapWidth, float characterSpacing, float lineSpacing) : alignment(alignment), offset(offset), wrapBefore(wrapBefore), wrapWidth(wrapWidth), characterSpacing(characterSpacing), lineSpacing(lineSpacing) { }
        };

        explicit TextFormatter(const std::shared_ptr<Font>& font);

        std::vector<Font::Glyph> format(const std::string& text, const Options& options) const;

    private:
        struct Line {
            cglib::bbox2<float> bbox;
            std::vector<Font::Glyph> glyphs;

            Line() : bbox(cglib::bbox2<float>::smallest()), glyphs() { }
        };

        std::vector<Line> splitLines(const std::string& text, const Options& options) const;

        std::shared_ptr<Font> _font;
    };
} }

#endif
