/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_TORQUEMARKERSYMBOLIZER_H_
#define _CARTO_MAPNIKVT_TORQUEMARKERSYMBOLIZER_H_

#include "Symbolizer.h"

namespace carto { namespace mvt {
    class TorqueMarkerSymbolizer : public Symbolizer {
    public:
        explicit TorqueMarkerSymbolizer(std::shared_ptr<Logger> logger) : Symbolizer(std::move(logger)) { }

        virtual void build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) override;

    protected:
        virtual void bindParameter(const std::string& name, const std::string& value) override;

        static std::shared_ptr<vt::Bitmap> makeEllipseBitmap(float width, float height, const vt::Color& color, float strokeWidth, const vt::Color& strokeColor);
        static std::shared_ptr<vt::Bitmap> makeRectangleBitmap(float width, float height, const vt::Color& color, float strokeWidth, const vt::Color& strokeColor);

        enum { DEFAULT_MARKER_SIZE = 10 };
        enum { SUPERSAMPLING_FACTOR = 4 };

        std::string _file;
        std::string _markerType;
        vt::Color _fill = vt::Color(0xff0000ff);
        float _fillOpacity = 1.0f;
        float _width = 10.0f;
        vt::Color _stroke = vt::Color(0xff000000);
        float _strokeOpacity = 1.0f;
        float _strokeWidth = 0.0f;
        std::string _compOp = "src-over";
    };
} }

#endif
