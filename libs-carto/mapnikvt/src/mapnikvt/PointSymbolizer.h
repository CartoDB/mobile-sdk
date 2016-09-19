/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_POINTSYMBOLIZER_H_
#define _CARTO_MAPNIKVT_POINTSYMBOLIZER_H_

#include "GeometrySymbolizer.h"

namespace carto { namespace mvt {
    class PointSymbolizer : public GeometrySymbolizer {
    public:
        explicit PointSymbolizer(std::shared_ptr<Logger> logger) : GeometrySymbolizer(std::move(logger)) { }

        virtual void build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) override;

    protected:
        constexpr static int RECTANGLE_SIZE = 4;

        virtual void bindParameter(const std::string& name, const std::string& value) override;

        static std::shared_ptr<vt::Bitmap> createRectangleBitmap(float size);

        std::string _file;
        vt::Color _fill = vt::Color(0xff000000);
        float _opacity = 1.0f;
        float _width = 10.0f;
        float _height = 10.0f;
        bool _allowOverlap = false;
        bool _ignorePlacement = false;
        cglib::mat3x3<float> _transform = cglib::mat3x3<float>::identity();
    };
} }

#endif
