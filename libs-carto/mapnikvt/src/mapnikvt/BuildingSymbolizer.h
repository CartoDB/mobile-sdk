/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_BUILDINGSYMBOLIZER_H_
#define _CARTO_MAPNIKVT_BUILDINGSYMBOLIZER_H_

#include "GeometrySymbolizer.h"

namespace carto { namespace mvt {
    class BuildingSymbolizer : public GeometrySymbolizer {
    public:
        explicit BuildingSymbolizer(std::shared_ptr<Logger> logger) : GeometrySymbolizer(std::move(logger)) {
            bind(&_fill, std::make_shared<ConstExpression>(Value(std::string("#808080"))), &BuildingSymbolizer::convertColor);
            bind(&_fillOpacity, std::make_shared<ConstExpression>(Value(1.0f)));
        }

        virtual void build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) override;

    protected:
        virtual void bindParameter(const std::string& name, const std::string& value) override;

        constexpr static float HEIGHT_SCALE = static_cast<float>(0.5 / 20037508.34);

        std::shared_ptr<const vt::ColorFunction> _fill; // vt::Color(0xff808080)
        std::shared_ptr<const vt::FloatFunction> _fillOpacity; // 1.0f
        float _height = 0.0f;
    };
} }

#endif
