#include "LinePatternSymbolizer.h"

namespace carto { namespace mvt {
    void LinePatternSymbolizer::build(const FeatureCollection& featureCollection, const SymbolizerContext& symbolizerContext, const ExpressionContext& exprContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        float geomScale = symbolizerContext.getSettings().getGeometryScale();
        vt::CompOp compOp = convertCompOp(_compOp);
        
        std::shared_ptr<const vt::BitmapPattern> pattern = symbolizerContext.getBitmapManager()->loadBitmapPattern(_file, 0.5f, 1.0f);
        if (!pattern) {
            _logger->write(Logger::Severity::ERROR, "Failed to load line pattern bitmap " + _file);
            return;
        }
        
        vt::LineStyle style(compOp, vt::LineJoinMode::MITER, vt::LineCapMode::NONE, vt::blendColor(_fill, _opacity), pattern->bitmap->height * geomScale * 0.375f * 0.5f, symbolizerContext.getStrokeMap(), pattern, _geometryTransform);

        for (std::size_t index = 0; index < featureCollection.getSize(); index++) {
            if (auto lineGeometry = std::dynamic_pointer_cast<const LineGeometry>(featureCollection.getGeometry(index))) {
                layerBuilder.addLines(lineGeometry->getVerticesList(), style);
            }
            else if (auto polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(featureCollection.getGeometry(index))) {
                for (const auto& verticesList : polygonGeometry->getPolygonList()) {
                    layerBuilder.addLines(verticesList, style);
                }
            }
            else {
                _logger->write(Logger::Severity::WARNING, "Unsupported geometry for LinePatternSymbolizer");
            }
        }
    }

    void LinePatternSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "file") {
            bind(&_file, parseStringExpression(value));
        }
        else if (name == "fill") {
            bind(&_fill, parseStringExpression(value), &LinePatternSymbolizer::convertColor);
        }
        else if (name == "opacity") {
            bind(&_opacity, parseExpression(value));
        }
        else {
            GeometrySymbolizer::bindParameter(name, value);
        }
    }
} }
