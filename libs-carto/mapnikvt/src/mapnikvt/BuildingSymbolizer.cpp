#include "BuildingSymbolizer.h"
#include "ParserUtils.h"

namespace carto { namespace mvt {
    void BuildingSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        vt::Polygon3DStyle style(_fill, _fillOpacity, _geometryTransform);

        std::size_t featureIndex = 0;
        std::size_t geometryIndex = 0;
        std::shared_ptr<const PolygonGeometry> polygonGeometry;
        layerBuilder.addPolygons3D([&](long long& id, vt::TileLayerBuilder::VerticesList& verticesList) {
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
                    _logger->write(Logger::Severity::WARNING, "Unsupported geometry for BuildingSymbolizer");
                }
                featureIndex++;
                geometryIndex = 0;
            }
            return false;
        }, _height * HEIGHT_SCALE, style);
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
