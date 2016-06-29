/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_TILELAYERSTYLES_H_
#define _CARTO_VT_TILELAYERSTYLES_H_

#include "Color.h"
#include "Bitmap.h"
#include "Font.h"
#include "StrokeMap.h"
#include "GlyphMap.h"
#include "TextFormatter.h"

#include <memory>

#include <boost/optional.hpp>

#include <cglib/vec.h>
#include <cglib/mat.h>

namespace carto { namespace vt {
    enum class CompOp {
        SRC, SRC_OVER, SRC_IN, SRC_ATOP, 
        DST, DST_OVER, DST_IN, DST_ATOP,
        ZERO, PLUS, MINUS, MULTIPLY,
        DARKEN, LIGHTEN
    };
    
    enum class LabelOrientation {
        BILLBOARD2D, BILLBOARD3D, POINT, POINT_FLIPPING, LINE
    };

    enum class LineJoinMode {
        NONE, BEVEL, MITER, ROUND
    };

    enum class LineCapMode {
        NONE, SQUARE, ROUND
    };

    struct PointStyle {
        CompOp compOp;
        Color color;
        float size;
        std::shared_ptr<GlyphMap> glyphMap;
        std::shared_ptr<const Bitmap> bitmap;
        boost::optional<cglib::mat3x3<float>> transform;

        explicit PointStyle(CompOp compOp, const Color& color, float size, std::shared_ptr<GlyphMap> glyphMap, std::shared_ptr<const Bitmap> bitmap, const boost::optional<cglib::mat3x3<float>>& transform) : compOp(compOp), color(color), size(size), glyphMap(std::move(glyphMap)), bitmap(std::move(bitmap)), transform(transform) { }
    };

    struct LineStyle {
        CompOp compOp;
        LineJoinMode joinMode;
        LineCapMode capMode;
        Color color;
        float width;
        std::shared_ptr<StrokeMap> strokeMap;
        std::shared_ptr<const BitmapPattern> strokePattern;
        boost::optional<cglib::mat3x3<float>> transform;

        explicit LineStyle(CompOp compOp, LineJoinMode joinMode, LineCapMode capMode, const Color& color, float width, std::shared_ptr<StrokeMap> strokeMap, std::shared_ptr<const BitmapPattern> strokePattern, const boost::optional<cglib::mat3x3<float>>& transform) : compOp(compOp), joinMode(joinMode), capMode(capMode), color(color), width(width), strokeMap(std::move(strokeMap)), strokePattern(std::move(strokePattern)), transform(transform) { }
    };

    struct PolygonStyle {
        CompOp compOp;
        Color color;
        std::shared_ptr<const BitmapPattern> pattern;
        boost::optional<cglib::mat3x3<float>> transform;

        explicit PolygonStyle(CompOp compOp, const Color& color, std::shared_ptr<const BitmapPattern> pattern, const boost::optional<cglib::mat3x3<float>>& transform) : compOp(compOp), color(color), pattern(std::move(pattern)), transform(transform) { }
    };

    struct Polygon3DStyle {
        Color color;
        boost::optional<cglib::mat3x3<float>> transform;

        explicit Polygon3DStyle(const Color& color, const boost::optional<cglib::mat3x3<float>>& transform) : color(color), transform(transform) { }
    };

    struct BitmapLabelStyle {
        LabelOrientation placement;
        Color color;
        std::shared_ptr<Font> font;
        std::shared_ptr<const Bitmap> bitmap;
        cglib::mat3x3<float> transform;

        explicit BitmapLabelStyle(LabelOrientation placement, const Color& color, std::shared_ptr<Font> font, std::shared_ptr<const Bitmap> bitmap, const cglib::mat3x3<float>& transform) : placement(placement), color(color), font(std::move(font)), bitmap(std::move(bitmap)), transform(transform) { }
    };

    struct TextLabelStyle {
        LabelOrientation placement;
        TextFormatter::Options formatterOptions;
        std::shared_ptr<Font> font;
        float angle;
        float backgroundScale;
        cglib::vec2<float> backgroundOffset;
        std::shared_ptr<const Bitmap> backgroundBitmap;

        explicit TextLabelStyle(LabelOrientation placement, const TextFormatter::Options& formatterOptions, std::shared_ptr<Font> font, float angle, float backgroundScale, const cglib::vec2<float>& backgroundOffset, std::shared_ptr<const Bitmap> backgroundBitmap) : placement(placement), formatterOptions(formatterOptions), font(std::move(font)), angle(angle), backgroundScale(backgroundScale), backgroundOffset(backgroundOffset), backgroundBitmap(std::move(backgroundBitmap)) { }
    };
} }

#endif
