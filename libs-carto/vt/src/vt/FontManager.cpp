#include "FontManager.h"
#include "Font.h"
#include "GlyphMap.h"

#include <atomic>
#include <mutex>
#include <memory>
#include <array>
#include <map>
#include <unordered_map>

#undef FT2_BUILD_LIBRARY
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#include FT_STROKER_H

#include <hb.h>
#include <hb-ft.h>

extern "C" {
    hb_unicode_funcs_t *hb_ucdn_get_unicode_funcs(void);
}

namespace carto { namespace vt {
    class FontManagerLibrary {
    public:
        FontManagerLibrary() : _library(nullptr) {
            std::lock_guard<std::recursive_mutex> lock(_Mutex);

            FT_Init_FreeType(&_library);
        }

        ~FontManagerLibrary() {
            std::lock_guard<std::recursive_mutex> lock(_Mutex);

            FT_Done_FreeType(_library);
            _library = nullptr;
        }

        FT_Library getLibrary() const {
            return _library;
        }

        std::recursive_mutex& getMutex() const {
            return _Mutex;
        }

    private:
        FT_Library _library;
        static std::recursive_mutex _Mutex; // use global lock as harfbuzz is not thread-safe on every platform
    };

    std::recursive_mutex FontManagerLibrary::_Mutex;

    class FontManagerFont : public Font {
    public:
        explicit FontManagerFont(const std::shared_ptr<FontManagerLibrary>& library, int maxGlyphMapWidth, int maxGlyphMapHeight, const std::vector<unsigned char>* data, const FontManager::Parameters& params) : _parameters(params), _library(library), _renderScale(static_cast<float>(TARGET_DPI) / static_cast<float>(RENDER_DPI)), _glyphMap(maxGlyphMapWidth, maxGlyphMapHeight), _face(nullptr), _font(nullptr), _metrics(0, 0, 0) {
            std::lock_guard<std::recursive_mutex> lock(_library->getMutex());

            // Load FreeType font
            if (data) {
                int error = FT_New_Memory_Face(_library->getLibrary(), data->data(), data->size(), 0, &_face);
                if (error == 0) {
                    error = FT_Set_Char_Size(_face, 0, static_cast<int>(std::floor(params.size * 64.0f)), RENDER_DPI, RENDER_DPI);
                }
            }

            // Create HarfBuzz font
            if (_face) {
                _metrics.ascent = _face->size->metrics.ascender / 64.0f * _renderScale;
                _metrics.descent = _face->size->metrics.descender / 64.0f * _renderScale;
                _metrics.height = _face->size->metrics.height / 64.0f * _renderScale;

                _font = hb_ft_font_create(_face, nullptr);
                if (_font) {
                    hb_ft_font_set_funcs(_font);
                }
            }
            
            // Initialize HarfBuzz buffer for glyph shaping
            _buffer = hb_buffer_create();
            if (_buffer) {
                hb_buffer_set_unicode_funcs(_buffer, hb_ucdn_get_unicode_funcs());
            }
    
            // Initialize gamma correction table
            for (std::size_t i = 0; i < _gammaTable.size(); i++) {
                _gammaTable[i] = static_cast<std::uint8_t>(255.0f * std::pow(i / 255.0f, 1.0f));
            }
        }

        virtual ~FontManagerFont() {
            std::lock_guard<std::recursive_mutex> lock(_library->getMutex());
            
            if (_buffer) {
                hb_buffer_destroy(_buffer);
                _buffer = nullptr;
            }

            if (_font) {
                hb_font_destroy(_font);
                _font = nullptr;
            }

            if (_face) {
                FT_Done_Face(_face);
                _face = nullptr;
            }
        }

        const FontManager::Parameters& getParameters() const {
            return _parameters;
        }

        virtual const Metrics& getMetrics() const override {
            return _metrics;
        }

        virtual std::vector<Glyph> shapeGlyphs(const std::uint32_t* utf32Text, std::size_t size, bool rtl) const override {
            std::lock_guard<std::recursive_mutex> lock(_library->getMutex());

            // Find first font that covers all the characters. If not possible, use the last
            unsigned int fontId = 0;
            const FontManagerFont* font = nullptr;
            for (const FontManagerFont* currentFont = this; currentFont; fontId++) {
                if (currentFont->_font) {
                    font = currentFont;
                    hb_buffer_clear_contents(_buffer);
                    hb_buffer_add_utf32(_buffer, utf32Text, static_cast<unsigned int>(size), 0, static_cast<unsigned int>(size));
                    hb_buffer_set_direction(_buffer, rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
                    hb_buffer_guess_segment_properties(_buffer);
                    hb_shape(font->_font, _buffer, nullptr, 0);

                    unsigned int infoCount = 0;
                    const hb_glyph_info_t* info = hb_buffer_get_glyph_infos(_buffer, &infoCount);
                    bool allValid = std::all_of(info, info + infoCount, [](const hb_glyph_info_t& glyphInfo) { return glyphInfo.codepoint != 0; });
                    if (allValid) {
                        break;
                    }
                }

                currentFont = dynamic_cast<const FontManagerFont*>(currentFont->_parameters.baseFont.get());
            }
            if (!font) {
                return std::vector<Glyph>();
            }

            // Get glyph list and glyph positions
            unsigned int infoCount = 0;
            const hb_glyph_info_t* info = hb_buffer_get_glyph_infos(_buffer, &infoCount);
            unsigned int posCount = 0;
            const hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(_buffer, &posCount);

            // Copy glyphs, render/cache bitmaps
            std::vector<Glyph> glyphs;
            glyphs.reserve(infoCount);
            for (unsigned int i = 0; i < infoCount; i++) {
                if (info[i].codepoint != 0) { // ignore 'missing glyph' glyphs
                    CodePoint remappedCodePoint = info[i].codepoint | (fontId << 24);
                    auto it = _codePointGlyphMap.find(remappedCodePoint);
                    if (it == _codePointGlyphMap.end()) {
                        GlyphId glyphId = addFreeTypeGlyph(font->_face, info[i].codepoint);
                        if (!glyphId) {
                            continue;
                        }
                        it = _codePointGlyphMap.insert({ remappedCodePoint, glyphId }).first;
                    }
                    if (const Glyph* glyph = _glyphMap.getGlyph(it->second)) {
                        glyphs.push_back(*glyph);
                        if (i < posCount) {
                            glyphs.back().offset += cglib::vec2<float>(pos[i].x_offset / 64.0f * _renderScale, pos[i].y_offset / 64.0f * _renderScale);
                            glyphs.back().advance = cglib::vec2<float>(pos[i].x_advance / 64.0f * _renderScale, pos[i].y_advance / 64.0f * _renderScale);
                        }
                    }
                }
            }
            return glyphs;
        }

        virtual const Glyph* loadBitmapGlyph(const std::shared_ptr<const Bitmap>& bitmap) override {
            std::lock_guard<std::recursive_mutex> lock(_library->getMutex());
            if (!bitmap) {
                return _glyphMap.getGlyph(_codePointGlyphMap[0]);
            }

            // Try to use cached glyph
            auto it = _bitmapGlyphMap.find(bitmap);
            if (it != _bitmapGlyphMap.end()) {
                return _glyphMap.getGlyph(_codePointGlyphMap[it->second]);
            }

            // Must load/render new glyph
            CodePoint codePoint = static_cast<CodePoint>(_bitmapGlyphMap.size() + BITMAP_CODEPOINTS);
            GlyphId glyphId = _glyphMap.loadBitmapGlyph(bitmap, codePoint);
            _codePointGlyphMap[codePoint] = glyphId;

            // Cache the generated glyph
            _bitmapGlyphMap[bitmap] = codePoint;
            return _glyphMap.getGlyph(glyphId);
        }

        virtual std::shared_ptr<const BitmapPattern> getBitmapPattern() const override {
            return _glyphMap.getBitmapPattern();
        }

    private:
        constexpr static int TARGET_DPI = 60;
        constexpr static int RENDER_DPI = 120;

        GlyphId addFreeTypeGlyph(FT_Face face, CodePoint codePoint) const {
            int error = FT_Load_Glyph(face, codePoint, FT_LOAD_DEFAULT);
            if (error != 0) {
                return 0;
            }

            FT_Bitmap* haloBitmap = nullptr;
            int haloBitmapLeft = 0;
            int haloBitmapTop = 0;

            std::shared_ptr<FT_GlyphRec_> ft_glyphPtr;
            if (_parameters.haloSize > 0) {
                FT_Stroker stroker;
                error = FT_Stroker_New(_library->getLibrary(), &stroker);
                if (error == 0) {
                    FT_Stroker_Set(stroker, static_cast<int>(_parameters.haloSize * 64.0f / _renderScale), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
                    FT_Glyph ft_glyph;
                    error = FT_Get_Glyph(face->glyph, &ft_glyph);
                    if (error == 0) {
                        error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
                        if (error == 0) {
                            error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
                        }
                        if (error == 0) {
                            FT_BitmapGlyph ft_bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(ft_glyph);
                            haloBitmap = &ft_bitmap_glyph->bitmap;
                            haloBitmapLeft = ft_bitmap_glyph->left;
                            haloBitmapTop = ft_bitmap_glyph->top;
                        }
                        ft_glyphPtr.reset(ft_glyph, FT_Done_Glyph); // delay the glyph destruction, we will need its contents
                    }
                    FT_Stroker_Done(stroker);
                }
            }

            error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
            if (error != 0) {
                return 0;
            }

            FT_Bitmap* baseBitmap = &face->glyph->bitmap;
            int baseBitmapLeft = face->glyph->bitmap_left;
            int baseBitmapTop = face->glyph->bitmap_top;

            int left = baseBitmapLeft;
            int top = baseBitmapTop;
            int width = baseBitmap->width;
            int height = baseBitmap->rows;
            if (haloBitmap) {
                int haloBitmapRight = haloBitmapLeft + haloBitmap->width;
                int baseBitmapRight = baseBitmapLeft + baseBitmap->width;
                int haloBitmapBottom = haloBitmapTop - static_cast<int>(haloBitmap->rows);
                int baseBitmapBottom = baseBitmapTop - static_cast<int>(baseBitmap->rows);
                if (haloBitmapLeft <= baseBitmapLeft && haloBitmapTop >= baseBitmapTop && haloBitmapRight >= baseBitmapRight && haloBitmapBottom <= baseBitmapBottom) {
                    left = haloBitmapLeft;
                    top = haloBitmapTop;
                    width = haloBitmap->width;
                    height = haloBitmap->rows;
                } else {
                    haloBitmap = nullptr;
                }
            }

            std::vector<std::uint32_t> glyphBitmapData(width * height);
            if (haloBitmap) {
                blendFreeTypeBitmap(glyphBitmapData, width, haloBitmap, _parameters.haloColor, 0, 0);
                blendFreeTypeBitmap(glyphBitmapData, width, baseBitmap, _parameters.color, baseBitmapLeft - haloBitmapLeft, -baseBitmapTop + haloBitmapTop);
            }
            else {
                blendFreeTypeBitmap(glyphBitmapData, width, baseBitmap, _parameters.color, 0, 0);
            }
            std::shared_ptr<Bitmap> glyphBitmap = std::make_shared<Bitmap>(width, height, std::move(glyphBitmapData));

            cglib::vec2<float> size(static_cast<float>(width), static_cast<float>(height));
            cglib::vec2<float> offset(static_cast<float>(left), static_cast<float>(top - height));
            cglib::vec2<float> advance(face->glyph->advance.x / 64.0f, face->glyph->advance.y / 64.0f);
            return _glyphMap.loadBitmapGlyph(glyphBitmap, codePoint, size * _renderScale, offset * _renderScale, advance * _renderScale);
        }

        void blendFreeTypeBitmap(std::vector<std::uint32_t>& buffer, std::size_t width, FT_Bitmap* bitmap, const Color& color, int x0, int y0) const {
            std::array<std::uint8_t, 4> glyphColor = color.rgba8();
            for (unsigned int y = 0; y < bitmap->rows; y++) {
                for (unsigned int x = 0; x < bitmap->width; x++) {
                    int alpha = bitmap->buffer[y * std::abs(bitmap->pitch) + x];
                    if (alpha == 0) {
                        continue;
                    }
                    alpha = _gammaTable[alpha];
                    std::uint8_t* bufferColor = reinterpret_cast<std::uint8_t*>(&buffer[(y0 + y) * width + x0 + x]);
                    for (int c = 0; c < 4; c++) {
                        int comp1 = glyphColor[c] * (alpha + 1);
                        int comp2 = bufferColor[c] * (255 - alpha);
                        bufferColor[c] = static_cast<std::uint8_t>((comp1 + comp2) >> 8);
                    }
                }
            }
        }

        const FontManager::Parameters _parameters;
        const std::shared_ptr<FontManagerLibrary> _library;
        const float _renderScale;
        std::array<std::uint8_t, 256> _gammaTable;
        mutable GlyphMap _glyphMap;
        mutable std::unordered_map<CodePoint, GlyphId> _codePointGlyphMap;
        std::unordered_map<std::shared_ptr<const Bitmap>, CodePoint> _bitmapGlyphMap;
        FT_Face _face;
        hb_font_t* _font;
        hb_buffer_t* _buffer;
        Metrics _metrics;
    };

    class FontManager::Impl {
    public:
        explicit Impl(int maxGlyphMapWidth, int maxGlyphMapHeight) : _maxGlyphMapWidth(maxGlyphMapWidth), _maxGlyphMapHeight(maxGlyphMapHeight), _library(std::make_shared<FontManagerLibrary>()) { }

        void loadFontData(const std::vector<unsigned char>& data) {
            std::lock_guard<std::mutex> lock(_mutex);

            if (data.empty()) {
                return;
            }

            FontManagerLibrary library;
            FT_Face face;
            int error = FT_New_Memory_Face(library.getLibrary(), data.data(), data.size(), 0, &face);
            if (error != 0) {
                return;
            }
            std::string fullName, family, subFamily;
            for (unsigned int i = 0; i < FT_Get_Sfnt_Name_Count(face); i++) {
                FT_SfntName sfntName;
                error = FT_Get_Sfnt_Name(face, i, &sfntName);
                if (error != 0) {
                    continue;
                }
                std::string name = readSfntName(sfntName);
                if (name.empty()) {
                    continue;
                }
                switch (sfntName.name_id) {
                case TT_NAME_ID_FULL_NAME:
                    fullName = name;
                    break;
                case TT_NAME_ID_FONT_FAMILY:
                case TT_NAME_ID_PREFERRED_FAMILY:
                    family = name;
                    break;
                case TT_NAME_ID_FONT_SUBFAMILY:
                case TT_NAME_ID_PREFERRED_SUBFAMILY:
                    subFamily = name;
                    break;
                default:
                    break;
                }
            }
            if (!fullName.empty()) {
                _fontDataMap[fullName] = data;
            }
            if (!family.empty()) {
                if (!subFamily.empty()) {
                    _fontDataMap[family + " " + subFamily] = data;
                }
                else {
                    _fontDataMap[family] = data;
                }
            }
            FT_Done_Face(face);
        }

        std::shared_ptr<Font> getFont(const std::string& name, const Parameters& parameters) const {
            std::lock_guard<std::mutex> lock(_mutex);

            // Try to use already cached font
            auto fontIt = _fontMap.find(name);
            if (fontIt != _fontMap.end()) {
                for (const std::shared_ptr<FontManagerFont>& font : fontIt->second) {
                    if (font->getParameters().size == parameters.size && font->getParameters().color == parameters.color && font->getParameters().haloSize == parameters.haloSize && font->getParameters().haloColor == parameters.haloColor && font->getParameters().baseFont == parameters.baseFont) {
                        return font;
                    }
                }
            }

            // Check if we have font corresponding to the name
            auto fontDataIt = _fontDataMap.find(name);
            if (fontDataIt == _fontDataMap.end()) {
                return std::shared_ptr<Font>();
            }

            // Create new font
            auto font = std::make_shared<FontManagerFont>(_library, _maxGlyphMapWidth, _maxGlyphMapHeight, &fontDataIt->second, parameters);

            // Preload often-used characters
            std::vector<std::uint32_t> glyphPreloadTable;
            std::for_each(_glyphPreloadTable.begin(), _glyphPreloadTable.end(), [&glyphPreloadTable](char c) { glyphPreloadTable.push_back(c); });
            for (std::size_t i = 0; i < glyphPreloadTable.size(); i++) {
                font->shapeGlyphs(&glyphPreloadTable[i], 1, false);
            }

            // Cache the font
            _fontMap[name].push_back(font);
            return font;
        }

        std::shared_ptr<Font> getNullFont() const {
            std::lock_guard<std::mutex> lock(_mutex);

            if (!_nullFont) {
                _nullFont = std::make_shared<FontManagerFont>(_library, _maxGlyphMapWidth, _maxGlyphMapHeight, nullptr, Parameters(0, vt::Color(), 0, vt::Color(), std::shared_ptr<Font>()));
            }
            return _nullFont;
        }

    private:
        static std::string readSfntName(const FT_SfntName& sfntName) {
            static const std::pair<int, int> be16Encodings[] = {
                { TT_PLATFORM_APPLE_UNICODE, TT_APPLE_ID_DEFAULT },
                { TT_PLATFORM_APPLE_UNICODE, TT_APPLE_ID_UNICODE_1_1 },
                { TT_PLATFORM_APPLE_UNICODE, TT_APPLE_ID_ISO_10646 },
                { TT_PLATFORM_APPLE_UNICODE, TT_APPLE_ID_UNICODE_2_0 },
                { TT_PLATFORM_ISO, TT_ISO_ID_10646 },
                { TT_PLATFORM_MICROSOFT, TT_MS_ID_UNICODE_CS },
                { TT_PLATFORM_MICROSOFT, TT_MS_ID_SYMBOL_CS },
                { -1, -1 }
            };

            for (int i = 0; be16Encodings[i].first != -1; i++) {
                if (be16Encodings[i].first == sfntName.platform_id && be16Encodings[i].second == sfntName.encoding_id) {
                    std::string name;
                    for (unsigned int j = 0; j < sfntName.string_len; j += 2) {
                        const char* c = reinterpret_cast<const char*>(sfntName.string) + j;
                        if (c[0] != 0) {
                            return std::string(); // simply ignore complex names
                        }
                        name.append(1, c[1]);
                    }
                    return name;
                }
            }
            return std::string(reinterpret_cast<const char*>(sfntName.string), sfntName.string_len);
        }

        const std::string _glyphPreloadTable = " 0123456789abcdefghijklmnopqrstuvxyzwABCDEFGHIJKLMNOPQRSTUVXYZ-,.";
        const int _maxGlyphMapWidth;
        const int _maxGlyphMapHeight;
        std::map<std::string, std::vector<unsigned char>> _fontDataMap;
        std::shared_ptr<FontManagerLibrary> _library;
        mutable std::map<std::string, std::vector<std::shared_ptr<FontManagerFont>>> _fontMap;
        mutable std::shared_ptr<Font> _nullFont;
        mutable std::mutex _mutex;
    };

    FontManager::FontManager(int maxGlyphMapWidth, int maxGlyphMapHeight) : _impl(std::unique_ptr<Impl>(new Impl(maxGlyphMapWidth, maxGlyphMapHeight))) {
    }

    FontManager::~FontManager() {
    }

    void FontManager::loadFontData(const std::vector<unsigned char>& data) {
        _impl->loadFontData(data);
    }

    std::shared_ptr<Font> FontManager::getFont(const std::string& name, const Parameters& parameters) const {
        return _impl->getFont(name, parameters);
    }

    std::shared_ptr<Font> FontManager::getNullFont() const {
        return _impl->getNullFont();
    }
} }
