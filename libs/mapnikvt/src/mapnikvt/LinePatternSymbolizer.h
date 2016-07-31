/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_LINEPATTERNSYMBOLIZER_H_
#define _CARTO_MAPNIKVT_LINEPATTERNSYMBOLIZER_H_

#include "GeometrySymbolizer.h"

namespace carto { namespace mvt {
    class LinePatternSymbolizer : public GeometrySymbolizer {
    public:
        explicit LinePatternSymbolizer(std::shared_ptr<Logger> logger) : GeometrySymbolizer(std::move(logger)) {
            bind(&_fill, std::make_shared<ConstExpression>(Value(std::string("#ffffff"))), &LinePatternSymbolizer::convertColor);
            bind(&_opacity, std::make_shared<ConstExpression>(Value(1.0f)));
        }

        virtual void build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) override;

    protected:
        virtual void bindParameter(const std::string& name, const std::string& value) override;

        std::string _file;
        std::shared_ptr<const vt::ColorFunction> _fill; // vt::Color(0xffffffff)
        std::shared_ptr<const vt::FloatFunction> _opacity; // 1.0f
    };
} }

#endif
