#include "PolygonSymbolizer.h"
#include "ParserUtils.h"

namespace carto { namespace mvt {
    void PolygonSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        if (_fillOpacityFunc == vt::FloatFunction(0) || _fillFunc == vt::ColorFunction(vt::Color())) {
            return;
        }
        
        vt::CompOp compOp = convertCompOp(_compOp);

        vt::ColorFunction fillFunc = _functionBuilder.createColorOpacityFunction(_fillFunc, _fillOpacityFunc);

        vt::PolygonStyle style(compOp, fillFunc, std::shared_ptr<vt::BitmapPattern>(), _geometryTransform);

        std::size_t featureIndex = 0;
        std::size_t geometryIndex = 0;
        std::shared_ptr<const PolygonGeometry> polygonGeometry;
        layerBuilder.addPolygons([&](long long& id, vt::TileLayerBuilder::VerticesList& verticesList) {
            while (true) {
                if (polygonGeometry) {
                    if (geometryIndex < polygonGeometry->getPolygonList().size()) {
                        id = featureCollection.getLocalId(featureIndex);
                        verticesList = polygonGeometry->getPolygonList()[geometryIndex++];
                        return true;
                    }
                    featureIndex++;
                    geometryIndex = 0;
                }

                if (featureIndex >= featureCollection.size()) {
                    break;
                }
                polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(featureCollection.getGeometry(featureIndex));
                if (!polygonGeometry) {
                    _logger->write(Logger::Severity::WARNING, "Unsupported geometry for PolygonSymbolizer");
                    featureIndex++;
                }
            }
            return false;
        }, style);
    }

    void PolygonSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "fill") {
            bind(&_fillFunc, parseStringExpression(value), &PolygonSymbolizer::convertColor);
        }
        else if (name == "fill-opacity") {
            bind(&_fillOpacityFunc, parseExpression(value));
        }
        else {
            GeometrySymbolizer::bindParameter(name, value);
        }
    }
} }
