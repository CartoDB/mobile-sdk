#include "BuildingSymbolizer.h"
#include "ParserUtils.h"

namespace carto { namespace mvt {
    void BuildingSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        vt::Polygon3DStyle style(vt::blendColor(_fill, _fillOpacity), _geometryTransform);

        for (std::size_t index = 0; index < featureCollection.getSize(); index++) {
            if (auto polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(featureCollection.getGeometry(index))) {
                layerBuilder.addPolygons3D(polygonGeometry->getPolygonList(), _height * HEIGHT_SCALE, style);
            }
            else {
                _logger->write(Logger::Severity::WARNING, "Unsupported geometry for BuildingSymbolizer");
            }
        }
    }

    void BuildingSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "fill") {
            bind(&_fill, parseStringExpression(value), &BuildingSymbolizer::convertColor);
        }
        else if (name == "fill-opacity") {
            bind(&_fillOpacity, parseExpression(value));
        }
        else if (name == "height") {
            bind(&_height, parseExpression(value));
        }
        else {
            GeometrySymbolizer::bindParameter(name, value);
        }
    }
} }
