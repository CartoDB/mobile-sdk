/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_POLYGONSYMBOLIZER_H_
#define _CARTO_MAPNIKVT_POLYGONSYMBOLIZER_H_

#include "GeometrySymbolizer.h"

namespace carto { namespace mvt {
    class PolygonSymbolizer : public GeometrySymbolizer {
    public:
        explicit PolygonSymbolizer(std::shared_ptr<Logger> logger) : GeometrySymbolizer(std::move(logger)) {
            bind(&_fillFunc, std::make_shared<ConstExpression>(Value(std::string("#808080"))), &PolygonSymbolizer::convertColor);
        }

        virtual void build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) override;

    protected:
        virtual void bindParameter(const std::string& name, const std::string& value) override;

        std::shared_ptr<const vt::ColorFunction> _fillFunc; // vt::Color(0xff808080)
        std::shared_ptr<const vt::FloatFunction> _fillOpacityFunc; // 1.0f
    };
} }

#endif
