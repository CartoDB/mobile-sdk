#include "PolygonPatternSymbolizer.h"

namespace carto { namespace mvt {
    void PolygonPatternSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        std::shared_ptr<const vt::BitmapPattern> pattern = symbolizerContext.getBitmapManager()->loadBitmapPattern(_file, 0.75f, 0.75f);
        if (!pattern) {
            _logger->write(Logger::Severity::ERROR, "Failed to load polygon pattern bitmap " + _file);
            return;
        }

        vt::CompOp compOp = convertCompOp(_compOp);
        
        vt::PolygonStyle style(compOp, _fill, _opacity, pattern, _geometryTransform);

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
                    _logger->write(Logger::Severity::WARNING, "Unsupported geometry for PolygonPatternSymbolizer");
                }
                featureIndex++;
                geometryIndex = 0;
            }
            return false;
        }, style);
    }

    void PolygonPatternSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "file") {
            bind(&_file, parseStringExpression(value));
        }
        else if (name == "fill") {
            bind(&_fill, parseStringExpression(value), &PolygonPatternSymbolizer::convertColor);
        }
        else if (name == "opacity") {
            bind(&_opacity, parseExpression(value));
        }
        else {
            GeometrySymbolizer::bindParameter(name, value);
        }
    }
} }
