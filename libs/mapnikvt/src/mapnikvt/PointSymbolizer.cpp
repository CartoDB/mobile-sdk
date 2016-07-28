#include "PointSymbolizer.h"
#include "ParserUtils.h"
#include "vt/BitmapCanvas.h"

namespace carto { namespace mvt {
    void PointSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        std::string file = _file;
        float bitmapScale = 1.0f;
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
            bitmapScale = 4.0f / RECTANGLE_SIZE;
        }

        vt::BitmapLabelStyle style(vt::LabelOrientation::BILLBOARD_2D, _fill * _opacity, symbolizerContext.getFontManager()->getNullFont(), bitmap, _transform * cglib::scale3_matrix(cglib::vec3<float>(bitmapScale, bitmapScale, 1)));

        for (std::size_t index = 0; index < featureCollection.getSize(); index++) {
            if (auto pointGeometry = std::dynamic_pointer_cast<const PointGeometry>(featureCollection.getGeometry(index))) {
                for (const auto& vertex : pointGeometry->getVertices()) {
                    long long id = getBitmapId(featureCollection.getId(index), file);
                    layerBuilder.addBitmapLabel(id, 0, vertex, 0, style);
                }
            }
            else if (auto lineGeometry = std::dynamic_pointer_cast<const LineGeometry>(featureCollection.getGeometry(index))) {
                for (const auto& vertex : lineGeometry->getMidPoints()) {
                    long long id = getBitmapId(featureCollection.getId(index), file);
                    layerBuilder.addBitmapLabel(id, 0, vertex, 0, style);
                }
            }
            else if (auto polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(featureCollection.getGeometry(index))) {
                for (const auto& vertex : polygonGeometry->getSurfacePoints()) {
                    long long id = getBitmapId(featureCollection.getId(index), file);
                    layerBuilder.addBitmapLabel(id, 0, vertex, 0, style);
                }
            }
            else {
                _logger->write(Logger::Severity::WARNING, "Unsupported geometry for PointSymbolizer");
            }
        }
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
            Symbolizer::bindParameter(name, value);
        }
    }

    std::shared_ptr<vt::Bitmap> PointSymbolizer::createRectangleBitmap(float size) {
        int canvasSize = static_cast<int>(std::ceil(size));
        vt::BitmapCanvas canvas(canvasSize, canvasSize);
        canvas.drawRectangle(0, 0, size, size);
        return canvas.buildBitmap();
    }
} }
