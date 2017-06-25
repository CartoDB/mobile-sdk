/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_STYLES_H_
#define _CARTO_VT_STYLES_H_

#include "Color.h"
#include "Bitmap.h"
#include "Font.h"
#include "ViewState.h"
#include "StrokeMap.h"
#include "GlyphMap.h"
#include "TextFormatter.h"

#include <memory>
#include <functional>

#include <boost/optional.hpp>

#include <cglib/vec.h>
#include <cglib/mat.h>

namespace carto { namespace vt {
    using FloatFunction = std::function<float(const ViewState&)>;
    using ColorFunction = std::function<Color(const ViewState&)>;

    enum class CompOp {
        SRC, SRC_OVER, SRC_IN, SRC_ATOP, 
        DST, DST_OVER, DST_IN, DST_ATOP,
        ZERO, PLUS, MINUS, MULTIPLY, SCREEN,
        DARKEN, LIGHTEN
    };
    
    enum class LabelOrientation {
        BILLBOARD_2D, BILLBOARD_3D, POINT, POINT_FLIPPING, LINE
    };

    enum class PointOrientation {
        BILLBOARD_2D, BILLBOARD_3D, POINT
    };

    enum class LineJoinMode {
        NONE, BEVEL, MITER, ROUND
    };

    enum class LineCapMode {
        NONE, SQUARE, ROUND
    };

    struct PointStyle final {
        CompOp compOp;
        PointOrientation orientation;
        std::shared_ptr<const ColorFunction> colorFunc;
        std::shared_ptr<const FloatFunction> sizeFunc;
        std::shared_ptr<const BitmapImage> pointImage;
        boost::optional<cglib::mat3x3<float>> transform;

        explicit PointStyle(CompOp compOp, PointOrientation orientation, std::shared_ptr<const ColorFunction> colorFunc, std::shared_ptr<const FloatFunction> sizeFunc, std::shared_ptr<const BitmapImage> pointImage, const boost::optional<cglib::mat3x3<float>>& transform) : compOp(compOp), orientation(orientation), colorFunc(std::move(colorFunc)), sizeFunc(std::move(sizeFunc)), pointImage(std::move(pointImage)), transform(transform) { }
    };

    struct TextStyle final {
        CompOp compOp;
        PointOrientation orientation;
        std::shared_ptr<const ColorFunction> colorFunc;
        std::shared_ptr<const FloatFunction> sizeFunc;
        std::shared_ptr<const ColorFunction> haloColorFunc;
        std::shared_ptr<const FloatFunction> haloRadiusFunc;
        float angle;
        float backgroundScale;
        cglib::vec2<float> backgroundOffset;
        std::shared_ptr<const BitmapImage> backgroundImage;
        boost::optional<cglib::mat3x3<float>> transform;

        explicit TextStyle(CompOp compOp, PointOrientation orientation, std::shared_ptr<const ColorFunction> colorFunc, std::shared_ptr<const FloatFunction> sizeFunc, std::shared_ptr<const ColorFunction> haloColorFunc, std::shared_ptr<const FloatFunction> haloRadiusFunc, float angle, float backgroundScale, const cglib::vec2<float>& backgroundOffset, std::shared_ptr<const BitmapImage> backgroundImage, const boost::optional<cglib::mat3x3<float>>& transform) : compOp(compOp), orientation(orientation), colorFunc(std::move(colorFunc)), sizeFunc(std::move(sizeFunc)), haloColorFunc(std::move(haloColorFunc)), haloRadiusFunc(std::move(haloRadiusFunc)), angle(angle), backgroundScale(backgroundScale), backgroundOffset(backgroundOffset), backgroundImage(std::move(backgroundImage)), transform(transform) { }
    };

    struct LineStyle final {
        CompOp compOp;
        LineJoinMode joinMode;
        LineCapMode capMode;
        std::shared_ptr<const ColorFunction> colorFunc;
        std::shared_ptr<const FloatFunction> widthFunc;
        std::shared_ptr<const BitmapPattern> strokePattern;
        boost::optional<cglib::mat3x3<float>> transform;

        explicit LineStyle(CompOp compOp, LineJoinMode joinMode, LineCapMode capMode, std::shared_ptr<const ColorFunction> colorFunc, std::shared_ptr<const FloatFunction> widthFunc, std::shared_ptr<const BitmapPattern> strokePattern, const boost::optional<cglib::mat3x3<float>>& transform) : compOp(compOp), joinMode(joinMode), capMode(capMode), colorFunc(std::move(colorFunc)), widthFunc(std::move(widthFunc)), strokePattern(std::move(strokePattern)), transform(transform) { }
    };

    struct PolygonStyle final {
        CompOp compOp;
        std::shared_ptr<const ColorFunction> colorFunc;
        std::shared_ptr<const BitmapPattern> pattern;
        boost::optional<cglib::mat3x3<float>> transform;

        explicit PolygonStyle(CompOp compOp, std::shared_ptr<const ColorFunction> colorFunc, std::shared_ptr<const BitmapPattern> pattern, const boost::optional<cglib::mat3x3<float>>& transform) : compOp(compOp), colorFunc(std::move(colorFunc)), pattern(std::move(pattern)), transform(transform) { }
    };

    struct Polygon3DStyle final {
        std::shared_ptr<const ColorFunction> colorFunc;
        boost::optional<cglib::mat3x3<float>> transform;

        explicit Polygon3DStyle(std::shared_ptr<const ColorFunction> colorFunc, const boost::optional<cglib::mat3x3<float>>& transform) : colorFunc(std::move(colorFunc)), transform(transform) { }
    };

    struct BitmapLabelStyle final {
        LabelOrientation orientation;
        std::shared_ptr<const ColorFunction> colorFunc;
        std::shared_ptr<const FloatFunction> sizeFunc;
        std::shared_ptr<const BitmapImage> image;
        cglib::mat3x3<float> transform;

        explicit BitmapLabelStyle(LabelOrientation orientation, std::shared_ptr<const ColorFunction> colorFunc, std::shared_ptr<const FloatFunction> sizeFunc, std::shared_ptr<const BitmapImage> image, const cglib::mat3x3<float>& transform) : orientation(orientation), colorFunc(std::move(colorFunc)), sizeFunc(std::move(sizeFunc)), image(std::move(image)), transform(transform){ }
    };

    struct TextLabelStyle final {
        LabelOrientation orientation;
        std::shared_ptr<const ColorFunction> colorFunc;
        std::shared_ptr<const FloatFunction> sizeFunc;
        std::shared_ptr<const ColorFunction> haloColorFunc;
        std::shared_ptr<const FloatFunction> haloRadiusFunc;
        float angle;
        float backgroundScale;
        cglib::vec2<float> backgroundOffset;
        std::shared_ptr<const BitmapImage> backgroundImage;

        explicit TextLabelStyle(LabelOrientation orientation, std::shared_ptr<const ColorFunction> colorFunc, std::shared_ptr<const FloatFunction> sizeFunc, std::shared_ptr<const ColorFunction> haloColorFunc, std::shared_ptr<const FloatFunction> haloRadiusFunc, float angle, float backgroundScale, const cglib::vec2<float>& backgroundOffset, std::shared_ptr<const BitmapImage> backgroundImage) : orientation(orientation), colorFunc(std::move(colorFunc)), sizeFunc(std::move(sizeFunc)), haloColorFunc(std::move(haloColorFunc)), haloRadiusFunc(std::move(haloRadiusFunc)), angle(angle), backgroundScale(backgroundScale), backgroundOffset(backgroundOffset), backgroundImage(std::move(backgroundImage)) { }
    };
} }

#endif
