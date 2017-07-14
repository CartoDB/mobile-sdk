/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_POLYGONPATTERNSYMBOLIZER_H_
#define _CARTO_MAPNIKVT_POLYGONPATTERNSYMBOLIZER_H_

#include "GeometrySymbolizer.h"

namespace carto { namespace mvt {
    class PolygonPatternSymbolizer : public GeometrySymbolizer {
    public:
        explicit PolygonPatternSymbolizer(std::shared_ptr<Logger> logger) : GeometrySymbolizer(std::move(logger)) {
            bind(&_fillFunc, std::make_shared<ConstExpression>(Value(std::string("#ffffff"))), &PolygonPatternSymbolizer::convertColor);
            bind(&_opacityFunc, std::make_shared<ConstExpression>(Value(1.0f)));
        }

        virtual void build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) override;

    protected:
        constexpr static float PATTERN_SCALE = 0.75f;

        virtual void bindParameter(const std::string& name, const std::string& value) override;

        std::string _file;
        vt::ColorFunction _fillFunc; // vt::Color(0xffffffff)
        vt::FloatFunction _opacityFunc; // 1.0f
    };
} }

#endif
