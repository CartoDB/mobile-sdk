#include "MBVTTileReader.h"
#include "MBVTFeatureDecoder.h"
#include "ValueConverter.h"
#include "Expression.h"
#include "ExpressionContext.h"

namespace carto { namespace mvt {
    void MBVTTileReader::setLayerNameOverride(const std::string& name) {
        _layerNameOverride = name;
    }

    std::string MBVTTileReader::getLayerName(const std::shared_ptr<const Layer>& layer) const {
        return _layerNameOverride.empty() ? layer->getName() : _layerNameOverride;
    }

    std::shared_ptr<FeatureDecoder::FeatureIterator> MBVTTileReader::createFeatureIterator(const std::shared_ptr<const Layer>& layer, const std::shared_ptr<const Style>& style, const FeatureExpressionContext& exprContext) const {
        std::unordered_set<std::shared_ptr<const Expression>> fieldExprs = style->getReferencedFields(exprContext.getZoom());
        std::unordered_set<std::string> fields;
        std::for_each(fieldExprs.begin(), fieldExprs.end(), [&](const std::shared_ptr<const Expression>& expr) {
            fields.insert(ValueConverter<std::string>::convert(expr->evaluate(exprContext)));
        });
        return _featureDecoder.createLayerFeatureIterator(getLayerName(layer), fields);
    }
} }
