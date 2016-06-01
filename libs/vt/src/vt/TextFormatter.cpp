#include "TextFormatter.h"

#include <utility>
#include <algorithm>
#include <numeric>
#include <vector>

#include <cglib/vec.h>
#include <cglib/mat.h>
#include <cglib/bbox.h>

#include <utf8.h>

#include <bidi.h>

namespace carto { namespace vt {
    TextFormatter::TextFormatter(const std::shared_ptr<Font>& font) : _font(font) {
    }

    std::vector<Font::Glyph> TextFormatter::format(const std::string& text, const Options& options) const {
        // Split text into lines
        std::vector<Line> lines = splitLines(text, options);
            
        // Calculate full bounding box and per-line bounding boxes
        cglib::bbox2<float> textBBox = cglib::bbox2<float>::smallest();
        std::for_each(lines.begin(), lines.end(), [&textBBox](const Line& line) { textBBox.add(line.bbox); });

        // Merge line runs, add pseudo-glyphs for new-line offsets
        std::vector<Font::Glyph> glyphs;
        glyphs.reserve(text.size() + lines.size());
        for (const Line& line : lines) {
            float xoff = -textBBox.max(0) * (options.alignment(0) + 1.0f) * 0.5f + (textBBox.size()(0) - line.bbox.size()(0)) * 0.5f;
            float yoff = (textBBox.min(1) - _font->getMetrics().descent) * (-options.alignment(1) + 1.0f) * 0.5f + (line.bbox.min(1) - textBBox.min(1));
            glyphs.emplace_back(Font::Glyph(Font::CR_CODEPOINT, 0, 0, 0, 0, cglib::vec2<float>(0, 0), cglib::vec2<float>(0, 0), cglib::vec2<float>(xoff, yoff) + options.offset));
            glyphs.insert(glyphs.end(), line.glyphs.begin(), line.glyphs.end());
        }

        return glyphs;
    }

    std::vector<TextFormatter::Line> TextFormatter::splitLines(const std::string& text, const Options& options) const {
        std::vector<std::uint32_t> utf32Text;
        utf32Text.reserve(text.size());
        utf8::utf8to32(text.begin(), text.end(), std::back_inserter(utf32Text));
        if (utf32Text.empty()) {
            return std::vector<TextFormatter::Line>();
        }
        
        // Classify characters for BIDI algorithm
        std::vector<int> types(utf32Text.size());
        std::vector<int> levels(utf32Text.size());
        std::vector<int> reorderLevels(utf32Text.size());
        bidi_classify(&utf32Text[0], &types[0], static_cast<int>(utf32Text.size()), 0);
        
        // Split original texts into lines, which are split into runs of the same script.
        // Also do line wrapping at this stage.
        std::vector<Line> lines;
        for (std::size_t ich = 0; ich < utf32Text.size(); ) {
            int baseLevel = -1;
            int cchText = static_cast<int>(utf32Text.size() - ich);
            int cchPara = bidi_paragraph(&baseLevel, &types[ich], &levels[ich], cchText);
            while (cchPara > 0) {
                int cchLine = bidi_line(baseLevel, &utf32Text[ich], &types[ich], &levels[ich], &reorderLevels[ich], cchPara, 1, nullptr);
                cchPara -= cchLine;

                lines.emplace_back(Line());
                float lineWidth = 0.0f;
                std::vector<Font::Glyph> word;
                float wordWidth = 0.0f;
                while (cchLine > 0) {
                    int cchRun = bidi_run(&utf32Text[ich], &reorderLevels[ich], cchLine, nullptr);
                    cchLine -= cchRun;
                    
                    std::vector<Font::Glyph> glyphs = _font->shapeGlyphs(&utf32Text[ich], cchRun, false);
                    for (std::size_t i = 0; i < glyphs.size(); i++) {
                        Font::Glyph& glyph = glyphs[i];
                        if (glyph.advance(0) > 0 && glyph.advance(1) == 0) {
                            glyph.advance(0) += options.characterSpacing;
                        }
                        if (glyph.width == 0) {
                            glyph.codePoint = Font::SPACE_CODEPOINT;
                        }

                        word.push_back(glyph);
                        wordWidth += glyph.advance(0);

                        if (glyph.codePoint == Font::SPACE_CODEPOINT && i + 1 < glyphs.size() && options.wrapWidth > 0) {
                            if (lineWidth + wordWidth >= options.wrapWidth) {
                                if (options.wrapBefore) {
                                    lines.emplace_back(Line());
                                    lines.back().glyphs.insert(lines.back().glyphs.end(), word.begin(), word.end());
                                    lineWidth = wordWidth;
                                }
                                else {
                                    lines.back().glyphs.insert(lines.back().glyphs.end(), word.begin(), word.end());
                                    lines.emplace_back(Line());
                                    lineWidth = 0;
                                }
                                
                                word.clear();
                                wordWidth = 0;
                                continue;
                            }
                            
                            lines.back().glyphs.insert(lines.back().glyphs.end(), word.begin(), word.end());
                            lineWidth += wordWidth;
                            
                            word.clear();
                            wordWidth = 0;
                        }
                    }
                    
                    ich += cchRun;
                }
                lines.back().glyphs.insert(lines.back().glyphs.end(), word.begin(), word.end());
            }
        }

        // Calculate line bounding boxes
        cglib::vec2<float> pen(0, 0);
        for (Line& line : lines) {
            pen(0) = 0;
            for (const Font::Glyph& glyph : line.glyphs) {
                line.bbox.add(pen + cglib::vec2<float>(glyph.offset(0), -_font->getMetrics().ascent));
                line.bbox.add(pen + cglib::vec2<float>(glyph.offset(0) + glyph.size(0), -_font->getMetrics().descent));

                pen += glyph.advance;
            }
            pen(1) -= _font->getMetrics().height + options.lineSpacing;
        }

        return lines;
    }
} }
