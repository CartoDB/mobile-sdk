#include "PointSymbolizer.h"
#include "ParserUtils.h"
#include "vt/BitmapCanvas.h"

namespace carto { namespace mvt {
    void PointSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        if (_opacityFunc == vt::FloatFunction(0)) {
            return;
        }
        
        vt::CompOp compOp = convertCompOp(_compOp);
        
        float fontScale = symbolizerContext.getSettings().getFontScale();

        std::string file = _file;
        std::shared_ptr<const vt::BitmapImage> bitmapImage;
        if (!file.empty()) {
            bitmapImage = symbolizerContext.getBitmapManager()->loadBitmapImage(file, false);
            if (!bitmapImage) {
                _logger->write(Logger::Severity::ERROR, "Failed to load point bitmap " + _file);
                return;
            }
        }
        else {
            file = "__default_point.bmp";
            bitmapImage = symbolizerContext.getBitmapManager()->getBitmapImage(file);
            if (!bitmapImage) {
                bitmapImage = makeRectangleBitmap(RECTANGLE_SIZE);
                symbolizerContext.getBitmapManager()->storeBitmapImage(file, bitmapImage);
            }
        }

        vt::FloatFunction sizeFunc = _functionBuilder.createFloatFunction(fontScale);
        vt::ColorFunction fillFunc = _functionBuilder.createColorOpacityFunction(_functionBuilder.createColorFunction(vt::Color(1, 1, 1, 1)), _opacityFunc);

        vt::PointStyle pointStyle(compOp, vt::PointOrientation::BILLBOARD_2D, fillFunc, sizeFunc, bitmapImage, _transform);

        std::vector<std::pair<long long, vt::TileLayerBuilder::Vertex>> pointInfos;
        for (std::size_t index = 0; index < featureCollection.size(); index++) {
            long long localId = featureCollection.getLocalId(index);
            if (auto pointGeometry = std::dynamic_pointer_cast<const PointGeometry>(featureCollection.getGeometry(index))) {
                for (const auto& vertex : pointGeometry->getVertices()) {
                    pointInfos.emplace_back(localId, vertex);
                }
            }
            else if (auto lineGeometry = std::dynamic_pointer_cast<const LineGeometry>(featureCollection.getGeometry(index))) {
                for (const auto& vertex : lineGeometry->getMidPoints()) {
                    pointInfos.emplace_back(localId, vertex);
                }
            }
            else if (auto polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(featureCollection.getGeometry(index))) {
                for (const auto& vertex : polygonGeometry->getSurfacePoints()) {
                    pointInfos.emplace_back(localId, vertex);
                }
            }
            else {
                _logger->write(Logger::Severity::WARNING, "Unsupported geometry for PointSymbolizer");
            }
        }

        std::size_t pointInfoIndex = 0;
        layerBuilder.addPoints([&](long long& id, vt::TileLayerBuilder::Vertex& vertex) {
            if (pointInfoIndex >= pointInfos.size()) {
                return false;
            }
            id = pointInfos[pointInfoIndex].first;
            vertex = pointInfos[pointInfoIndex].second;
            pointInfoIndex++;
            return true;
        }, pointStyle, symbolizerContext.getGlyphMap());
    }

    void PointSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "file") {
            bind(&_file, parseStringExpression(value));
        }
        else if (name == "opacity") {
            bind(&_opacityFunc, parseExpression(value));
        }
        else if (name == "allow-overlap") {
            bind(&_allowOverlap, parseExpression(value));
        }
        else if (name == "ignore-placement") {
            bind(&_ignorePlacement, parseExpression(value));
        }
        else if (name == "transform") {
            bind(&_transform, parseStringExpression(value), &PointSymbolizer::convertTransform);
        }
        else {
            GeometrySymbolizer::bindParameter(name, value);
        }
    }

    std::shared_ptr<vt::BitmapImage> PointSymbolizer::makeRectangleBitmap(float size) {
        int canvasSize = static_cast<int>(std::ceil(size));
        vt::BitmapCanvas canvas(canvasSize, canvasSize, false);
        canvas.setColor(vt::Color(0, 0, 0, 1));
        canvas.drawRectangle(0, 0, size, size);
        return canvas.buildBitmapImage();
    }
} }
