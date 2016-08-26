#include "PointSymbolizer.h"
#include "ParserUtils.h"
#include "vt/BitmapCanvas.h"

namespace carto { namespace mvt {
    void PointSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        vt::CompOp compOp = convertCompOp(_compOp);

        float fontScale = symbolizerContext.getSettings().getFontScale();
        float bitmapScale = fontScale;
        std::string file = _file;
        std::shared_ptr<const vt::Bitmap> bitmap;
        if (!file.empty()) {
            bitmap = symbolizerContext.getBitmapManager()->loadBitmap(file);
            if (!bitmap) {
                _logger->write(Logger::Severity::ERROR, "Failed to load point bitmap " + _file);
                return;
            }
        }
        else {
            file = "__default_point.bmp";
            bitmap = symbolizerContext.getBitmapManager()->getBitmap(file);
            if (!bitmap) {
                bitmap = createRectangleBitmap(RECTANGLE_SIZE);
                symbolizerContext.getBitmapManager()->storeBitmap(file, bitmap);
            }
            bitmapScale = fontScale * 4.0f / RECTANGLE_SIZE;
        }

        std::shared_ptr<const vt::FloatFunction> widthFunc;
        ExpressionFunctionBinder<float>().bind(&widthFunc, std::make_shared<ConstExpression>(Value(_width * fontScale))).update(exprContext);
        std::shared_ptr<const vt::ColorFunction> fillFunc;
        ExpressionFunctionBinder<vt::Color>().bind(&fillFunc, std::make_shared<ConstExpression>(Value(std::string("#ffffff"))), [this](const Value& val) -> vt::Color {
            return convertColor(val);
        }).update(exprContext);
        std::shared_ptr<const vt::FloatFunction> opacityFunc;
        ExpressionFunctionBinder<float>().bind(&opacityFunc, std::make_shared<ConstExpression>(Value(_opacity))).update(exprContext);

        vt::PointStyle pointStyle(compOp, vt::PointOrientation::BILLBOARD_2D, fillFunc, opacityFunc, widthFunc, symbolizerContext.getGlyphMap(), bitmap, _transform * cglib::scale3_matrix(cglib::vec3<float>(1.0f, (bitmap->height * bitmapScale) / (bitmap->width * bitmapScale), 1)));

        std::vector<std::pair<long long, vt::TileLayerBuilder::Vertex>> pointInfos;
        for (std::size_t index = 0; index < featureCollection.getSize(); index++) {
            if (auto pointGeometry = std::dynamic_pointer_cast<const PointGeometry>(featureCollection.getGeometry(index))) {
                for (const auto& vertex : pointGeometry->getVertices()) {
                    pointInfos.emplace_back(featureCollection.getId(index), vertex);
                }
            }
            else if (auto lineGeometry = std::dynamic_pointer_cast<const LineGeometry>(featureCollection.getGeometry(index))) {
                for (const auto& vertex : lineGeometry->getMidPoints()) {
                    pointInfos.emplace_back(featureCollection.getId(index), vertex);
                }
            }
            else if (auto polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(featureCollection.getGeometry(index))) {
                for (const auto& vertex : polygonGeometry->getSurfacePoints()) {
                    pointInfos.emplace_back(featureCollection.getId(index), vertex);
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
        }, pointStyle);
    }

    void PointSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "file") {
            bind(&_file, parseStringExpression(value));
        }
        else if (name == "opacity") {
            bind(&_opacity, parseExpression(value));
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

    std::shared_ptr<vt::Bitmap> PointSymbolizer::createRectangleBitmap(float size) {
        int canvasSize = static_cast<int>(std::ceil(size));
        vt::BitmapCanvas canvas(canvasSize, canvasSize);
        canvas.drawRectangle(0, 0, size, size);
        return canvas.buildBitmap();
    }
} }
