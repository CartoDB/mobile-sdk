#include "PolygonPatternSymbolizer.h"

namespace carto { namespace mvt {
    void PolygonPatternSymbolizer::build(const FeatureCollection& featureCollection, const SymbolizerContext& symbolizerContext, const ExpressionContext& exprContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        std::shared_ptr<const vt::BitmapPattern> pattern = symbolizerContext.getBitmapManager()->loadBitmapPattern(_file, 0.75f, 0.75f);
        if (!pattern) {
            _logger->write(Logger::Severity::ERROR, "Failed to load polygon pattern bitmap " + _file);
            return;
        }

        vt::CompOp compOp = convertCompOp(_compOp);
        
        vt::PolygonStyle style(compOp, vt::blendColor(_fill, _opacity), pattern, _geometryTransform);

        for (std::size_t index = 0; index < featureCollection.getSize(); index++) {
            if (auto polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(featureCollection.getGeometry(index))) {
                layerBuilder.addPolygons(polygonGeometry->getPolygonList(), style);
            }
            else {
                _logger->write(Logger::Severity::WARNING, "Unsupported geometry for PolygonPatternSymbolizer");
            }
        }
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
