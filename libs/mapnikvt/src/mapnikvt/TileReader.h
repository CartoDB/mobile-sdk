/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_TILEREADER_H_
#define _CARTO_MAPNIKVT_TILEREADER_H_

#include "FeatureDecoder.h"
#include "vt/Tile.h"
#include "vt/TileLayerBuilder.h"

#include <memory>

#include <cglib/vec.h>
#include <cglib/mat.h>

namespace carto { namespace mvt {
    class Map;
    class Filter;
    class Rule;
    class Expression;
    class ExpressionContext;
    class Symbolizer;
    class SymbolizerContext;
    class Layer;
    class Style;
    
    class TileReader {
    public:
        virtual ~TileReader() = default;

        virtual std::shared_ptr<vt::Tile> readTile(const vt::TileId& tileId) const;

    protected:
        explicit TileReader(std::shared_ptr<Map> map, const SymbolizerContext& symbolizerContext);

        void processLayer(const std::shared_ptr<Layer>& layer, const std::shared_ptr<Style>& style, ExpressionContext& exprContext, vt::TileLayerBuilder& layerBuilder) const;

        std::vector<std::shared_ptr<Symbolizer>> findFeatureSymbolizers(const std::shared_ptr<Style>& style, ExpressionContext& exprContext) const;
        
        virtual std::shared_ptr<FeatureDecoder::FeatureIterator> createFeatureIterator(const std::shared_ptr<Layer>& layer, const std::shared_ptr<Style>& style, const ExpressionContext& exprContext) const = 0;

        const std::shared_ptr<Map> _map;
        const SymbolizerContext& _symbolizerContext;
        const std::shared_ptr<Filter> _trueFilter;
    };
} }

#endif
