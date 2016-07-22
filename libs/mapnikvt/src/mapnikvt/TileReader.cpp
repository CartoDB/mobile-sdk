#include "TileReader.h"
#include "ParserUtils.h"
#include "Symbolizer.h"
#include "Predicate.h"
#include "Expression.h"
#include "ExpressionContext.h"
#include "Rule.h"
#include "Filter.h"
#include "Map.h"

namespace carto { namespace mvt {
    TileReader::TileReader(std::shared_ptr<Map> map, const SymbolizerContext& symbolizerContext) :
        _map(std::move(map)), _symbolizerContext(symbolizerContext), _trueFilter(std::make_shared<Filter>(Filter::Type::FILTER, std::make_shared<ConstPredicate>(true)))
    {
    }

    std::shared_ptr<vt::Tile> TileReader::readTile(const vt::TileId& tileId) const {
        FeatureExpressionContext exprContext;
        exprContext.setZoom(tileId.zoom + static_cast<int>(_symbolizerContext.getSettings().getZoomLevelBias()));
        exprContext.setNutiParameterValueMap(_symbolizerContext.getSettings().getNutiParameterValueMap());
        vt::TileLayerBuilder tileLayerBuilder(_symbolizerContext.getSettings().getTileSize(), _symbolizerContext.getSettings().getGeometryScale());

        std::vector<std::shared_ptr<vt::TileLayer>> tileLayers;
        int layerIdx = 0;
        for (const std::shared_ptr<Layer>& layer : _map->getLayers()) {
            int styleIdx = 0;
            for (const std::string& styleName : layer->getStyleNames()) {
                const std::shared_ptr<Style>& style = _map->getStyle(styleName);
                if (!style) {
                    continue;
                }
                
                processLayer(layer, style, exprContext, tileLayerBuilder);

                boost::optional<vt::CompOp> compOp;
                try {
                    if (!style->getCompOp().empty()) {
                        compOp = parseCompOp(style->getCompOp());
                    }
                }
                catch (const ParserException&) {
                    // ignore the error
                }
                
                std::shared_ptr<vt::TileLayer> tileLayer = tileLayerBuilder.build(layerIdx * 65536 + styleIdx, style->getOpacity(), compOp);
                if (!(tileLayer->getBitmaps().empty() && tileLayer->getLabels().empty() && tileLayer->getGeometries().empty() && !compOp)) {
                    tileLayers.push_back(tileLayer);
                }
                styleIdx++;
            }
            layerIdx++;
        }
        return std::make_shared<vt::Tile>(tileId, tileLayers);
    }

    void TileReader::processLayer(const std::shared_ptr<Layer>& layer, const std::shared_ptr<Style>& style, FeatureExpressionContext& exprContext, vt::TileLayerBuilder& layerBuilder) const {
        std::shared_ptr<Symbolizer> currentSymbolizer;
        FeatureCollection currentFeatureCollection;
        std::unordered_map<std::shared_ptr<FeatureData>, std::vector<std::shared_ptr<Symbolizer>>> featureDataSymbolizersMap;
        if (auto featureIt = createFeatureIterator(layer, style, exprContext)) {
            for (; featureIt->valid(); featureIt->advance()) {
                // Cache symbolizer evaluation for each feature data object
                std::shared_ptr<FeatureData> featureData = featureIt->getFeatureData();
                auto symbolizersIt = featureDataSymbolizersMap.find(featureData);
                if (symbolizersIt == featureDataSymbolizersMap.end()) {
                    exprContext.setFeatureData(featureData);
                    std::vector<std::shared_ptr<Symbolizer>> symbolizers = findFeatureSymbolizers(style, exprContext);
                    symbolizersIt = featureDataSymbolizersMap.emplace(featureData, std::move(symbolizers)).first;
                }

                // Process symbolizers, try to batch as many calls together as possible
                for (const std::shared_ptr<Symbolizer>& symbolizer : symbolizersIt->second) {
                    if (std::shared_ptr<Geometry> geometry = featureIt->getGeometry()) {
                        bool batch = false;
                        if (currentSymbolizer == symbolizer) {
                            if (currentFeatureCollection.getFeatureData() == featureData || symbolizer->getParameterExpressions().empty()) {
                                batch = true;
                            }
                        }

                        if (!batch) {
                            if (currentSymbolizer) {
                                exprContext.setFeatureData(currentFeatureCollection.getFeatureData());
                                currentSymbolizer->build(currentFeatureCollection, exprContext, _symbolizerContext, layerBuilder);
                            }
                            currentFeatureCollection.clear();
                            currentFeatureCollection.setFeatureData(featureData);
                            currentSymbolizer = symbolizer;
                        }

                        currentFeatureCollection.append(featureIt->getFeatureId(), geometry);
                    }
                }
            }

            // Flush the remaining batched features
            if (currentSymbolizer) {
                exprContext.setFeatureData(currentFeatureCollection.getFeatureData());
                currentSymbolizer->build(currentFeatureCollection, exprContext, _symbolizerContext, layerBuilder);
            }
        }
    }

    std::vector<std::shared_ptr<Symbolizer>> TileReader::findFeatureSymbolizers(const std::shared_ptr<Style>& style, FeatureExpressionContext& exprContext) const {
        bool anyMatch = false;
        std::vector<std::shared_ptr<Symbolizer>> symbolizers;
        for (const std::shared_ptr<const Rule>& rule : style->getZoomRules(exprContext.getZoom())) {
            std::shared_ptr<const Filter> filter = rule->getFilter();
            if (!filter) {
                filter = _trueFilter;
            }
            
            // Filter matching logic
            bool match = true;
            switch (filter->getType()) {
            case Filter::Type::FILTER:
                switch (style->getFilterMode()) {
                case Style::FilterMode::FIRST:
                    if (anyMatch) {
                        match = false;
                    }
                    else if (const std::shared_ptr<const Predicate>& pred = filter->getPredicate()) {
                        match = pred->evaluate(exprContext);
                    }
                    break;
                case Style::FilterMode::ALL:
                    if (const std::shared_ptr<const Predicate>& pred = filter->getPredicate()) {
                        match = pred->evaluate(exprContext);
                    }
                    break;
                }
                anyMatch = anyMatch || match;
                break;
            case Filter::Type::ELSEFILTER:
                match = !anyMatch;
                break;
            case Filter::Type::ALSOFILTER:
                match = anyMatch;
                break;
            }

            // If match, add all rule symbolizers to the symbolizer list
            if (match) {
                symbolizers.insert(symbolizers.end(), rule->getSymbolizers().begin(), rule->getSymbolizers().end());
            }
        }
        return symbolizers;
    }
} }
