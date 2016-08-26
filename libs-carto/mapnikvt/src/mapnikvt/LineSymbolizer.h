/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_LINESYMBOLIZER_H_
#define _CARTO_MAPNIKVT_LINESYMBOLIZER_H_

#include "GeometrySymbolizer.h"

namespace carto { namespace mvt {
    class LineSymbolizer : public GeometrySymbolizer {
    public:
        explicit LineSymbolizer(std::shared_ptr<Logger> logger) : GeometrySymbolizer(std::move(logger)) {
            bind(&_stroke, std::make_shared<ConstExpression>(Value(std::string("#000000"))), &LineSymbolizer::convertColor);
            bind(&_strokeWidth, std::make_shared<ConstExpression>(Value(1.0f)));
            bind(&_strokeOpacity, std::make_shared<ConstExpression>(Value(1.0f)));
        }

        virtual void build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) override;

    protected:
        vt::LineCapMode convertLineCapMode(const std::string& lineCap) const;
        vt::LineJoinMode convertLineJoinMode(const std::string& lineJoin) const;

        virtual void bindParameter(const std::string& name, const std::string& value) override;

        static std::shared_ptr<vt::BitmapPattern> createDashBitmapPattern(const std::vector<float>& strokeDashArray);

        enum { MIN_SUPERSAMPLING_FACTOR = 2, MAX_SUPERSAMPLING_FACTOR = 16 };

        std::shared_ptr<const vt::ColorFunction> _stroke; // vt::Color(0xff000000)
        std::shared_ptr<const vt::FloatFunction> _strokeWidth; // 1.0f
        std::shared_ptr<const vt::FloatFunction> _strokeOpacity; // 1.0f
        std::string _strokeLinejoin = "miter";
        std::string _strokeLinecap = "butt";
        std::string _strokeDashArray;
    };
} }

#endif
