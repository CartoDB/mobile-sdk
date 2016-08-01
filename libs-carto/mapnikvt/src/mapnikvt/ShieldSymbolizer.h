/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_SHIELDSYMBOLIZER_H_
#define _CARTO_MAPNIKVT_SHIELDSYMBOLIZER_H_

#include "TextSymbolizer.h"

namespace carto { namespace mvt {
    class ShieldSymbolizer : public TextSymbolizer {
    public:
        explicit ShieldSymbolizer(std::vector<std::shared_ptr<FontSet>> fontSets, std::shared_ptr<Logger> logger) : TextSymbolizer(std::move(fontSets), std::move(logger)) { }

        virtual void build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) override;

    protected:
        virtual void bindParameter(const std::string& name, const std::string& value) override;

        std::string _file;
        bool _unlockImage = false;
        float _shieldDx = 0.0f;
        float _shieldDy = 0.0f;
    };
} }

#endif
