#include "PolygonSymbolizer.h"
#include "ParserUtils.h"

namespace carto { namespace mvt {
    void PolygonSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        vt::CompOp compOp = convertCompOp(_compOp);
        
        vt::PolygonStyle style(compOp, _fill, _fillOpacity, std::shared_ptr<vt::BitmapPattern>(), _geometryTransform);

        std::size_t featureIndex = 0;
        std::size_t geometryIndex = 0;
        std::shared_ptr<const PolygonGeometry> polygonGeometry;
        layerBuilder.addPolygons([&](long long& id, vt::TileLayerBuilder::VerticesList& verticesList) {
            while (true) {
                if (polygonGeometry) {
                    if (geometryIndex < polygonGeometry->getPolygonList().size()) {
                        verticesList = polygonGeometry->getPolygonList()[geometryIndex++];
                        return true;
                    }
                }

                if (featureIndex >= featureCollection.getSize()) {
                    break;
                }
                polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(featureCollection.getGeometry(featureIndex));
                if (!polygonGeometry) {
                    _logger->write(Logger::Severity::WARNING, "Unsupported geometry for PolygonSymbolizer");
                }
                featureIndex++;
                geometryIndex = 0;
            }
            return false;
        }, style);
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
