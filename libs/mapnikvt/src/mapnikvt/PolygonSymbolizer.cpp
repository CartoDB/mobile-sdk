#include "PolygonSymbolizer.h"
#include "ParserUtils.h"

namespace carto { namespace mvt {
    void PolygonSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        vt::CompOp compOp = convertCompOp(_compOp);
        
        vt::PolygonStyle style(compOp, _fill, _fillOpacity, std::shared_ptr<vt::BitmapPattern>(), _geometryTransform);

        for (std::size_t index = 0; index < featureCollection.getSize(); index++) {
            if (auto polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(featureCollection.getGeometry(index))) {
                layerBuilder.addPolygons(polygonGeometry->getPolygonList(), style);
            }
            else {
                _logger->write(Logger::Severity::WARNING, "Unsupported geometry for PolygonSymbolizer");
            }
        }
    }

    void PolygonSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "fill") {
            bind(&_fill, parseStringExpression(value), &PolygonSymbolizer::convertColor);
        }
        else if (name == "fill-opacity") {
            bind(&_fillOpacity, parseExpression(value));
        }
        else {
            GeometrySymbolizer::bindParameter(name, value);
        }
    }
} }
