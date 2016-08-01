#include "TorqueMarkerSymbolizer.h"
#include "ParserUtils.h"
#include "vt/BitmapCanvas.h"

namespace carto { namespace mvt {
    void TorqueMarkerSymbolizer::build(const FeatureCollection& featureCollection, const SymbolizerContext& symbolizerContext, const ExpressionContext& exprContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        float geomScale = symbolizerContext.getSettings().getGeometryScale();
        vt::CompOp compOp = convertCompOp(_compOp);

        float width = DEFAULT_MARKER_SIZE, height = DEFAULT_MARKER_SIZE;
        if (_width > 0) {
            width = height = _width;
        }

        std::shared_ptr<const vt::Bitmap> bitmap;
        vt::Color fill = vt::blendColor(_fill, _fillOpacity);
        vt::Color stroke = vt::blendColor(_stroke, _strokeOpacity);
        if (!_file.empty()) {
            bitmap = symbolizerContext.getBitmapManager()->loadBitmap(_file);
            if (!bitmap) {
                _logger->write(Logger::Severity::ERROR, "Failed to load marker bitmap " + _file);
                return;
            }
            fill = vt::blendColor(vt::Color(0xffffffff), _fillOpacity);
            width = bitmap->width;
            height = bitmap->height;
        }
        else {
            if (_markerType == "rectangle") {
                std::string file = "__torque_marker_rectangle_" + boost::lexical_cast<std::string>(width) + "_" + boost::lexical_cast<std::string>(height) + "_" + boost::lexical_cast<std::string>(fill.value()) + "_" + boost::lexical_cast<std::string>(_strokeWidth) + "_" + boost::lexical_cast<std::string>(stroke.value()) + ".bmp";
                bitmap = symbolizerContext.getBitmapManager()->getBitmap(file);
                if (!bitmap) {
                    bitmap = makeRectangleBitmap(width * SUPERSAMPLING_FACTOR, height * SUPERSAMPLING_FACTOR, fill, _strokeWidth * SUPERSAMPLING_FACTOR, stroke);
                    symbolizerContext.getBitmapManager()->storeBitmap(file, bitmap);
                }
                fill = vt::Color(0xffffffff);
            }
            else {
                std::string file = "__torque_marker_ellipse_" + boost::lexical_cast<std::string>(width) + "_" + boost::lexical_cast<std::string>(height) + "_" + boost::lexical_cast<std::string>(fill.value()) + "_" + boost::lexical_cast<std::string>(_strokeWidth) + "_" + boost::lexical_cast<std::string>(stroke.value()) + ".bmp";
                bitmap = symbolizerContext.getBitmapManager()->getBitmap(file);
                if (!bitmap) {
                    bitmap = makeEllipseBitmap(width * SUPERSAMPLING_FACTOR, height * SUPERSAMPLING_FACTOR, fill, _strokeWidth * SUPERSAMPLING_FACTOR, stroke);
                    symbolizerContext.getBitmapManager()->storeBitmap(file, bitmap);
                }
                fill = vt::Color(0xffffffff);
            }
        }

        vt::PointStyle style(compOp, fill, width * geomScale, symbolizerContext.getGlyphMap(), bitmap, boost::optional<cglib::mat3x3<float>>());

        for (std::size_t index = 0; index < featureCollection.getSize(); index++) {
            if (auto pointGeometry = std::dynamic_pointer_cast<const PointGeometry>(featureCollection.getGeometry(index))) {
                layerBuilder.addPoints(pointGeometry->getVertices(), style);
            }
            else {
                _logger->write(Logger::Severity::WARNING, "Unsupported geometry for TorqueMarkerSymbolizer");
            }
        }
    }

    void TorqueMarkerSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "file") {
            bind(&_file, parseStringExpression(value));
        }
        else if (name == "marker-type") {
            bind(&_markerType, parseStringExpression(value));
        }
        else if (name == "fill") {
            bind(&_fill, parseStringExpression(value), &TorqueMarkerSymbolizer::convertColor);
        }
        else if (name == "fill-opacity") {
            bind(&_fillOpacity, parseExpression(value));
        }
        else if (name == "width") {
            bind(&_width, parseExpression(value));
        }
        else if (name == "stroke") {
            bind(&_stroke, parseStringExpression(value), &TorqueMarkerSymbolizer::convertColor);
        }
        else if (name == "stroke-opacity") {
            bind(&_strokeOpacity, parseExpression(value));
        }
        else if (name == "stroke-width") {
            bind(&_strokeWidth, parseExpression(value));
        }
        else if (name == "comp-op") {
            bind(&_compOp, parseStringExpression(value));
        }
        else if (name == "opacity") { // binds to 2 parameters
            bind(&_fillOpacity, parseExpression(value));
            bind(&_strokeOpacity, parseExpression(value));
        }
        else {
            Symbolizer::bindParameter(name, value);
        }
    }

    std::shared_ptr<vt::Bitmap> TorqueMarkerSymbolizer::makeEllipseBitmap(float width, float height, const vt::Color& color, float strokeWidth, const vt::Color& strokeColor) {
        int canvasWidth = static_cast<int>(std::ceil(width + strokeWidth));
        int canvasHeight = static_cast<int>(std::ceil(height + strokeWidth));
        vt::BitmapCanvas canvas(canvasWidth, canvasHeight);
        float x0 = canvasWidth * 0.5f, y0 = canvasHeight * 0.5f;
        if (strokeWidth > 0) {
            canvas.setColor(strokeColor);
            canvas.drawEllipse(x0, y0, (width + strokeWidth * 0.5f) * 0.5f, (height + strokeWidth * 0.5f) * 0.5f);
        }
        canvas.setColor(color);
        canvas.drawEllipse(x0, y0, (width - strokeWidth * 0.5f) * 0.5f, (height - strokeWidth * 0.5f) * 0.5f);
        return canvas.buildBitmap();
    }

    std::shared_ptr<vt::Bitmap> TorqueMarkerSymbolizer::makeRectangleBitmap(float width, float height, const vt::Color& color, float strokeWidth, const vt::Color& strokeColor) {
        int canvasWidth = static_cast<int>(std::ceil(width + strokeWidth));
        int canvasHeight = static_cast<int>(std::ceil(height + strokeWidth));
        vt::BitmapCanvas canvas(canvasWidth, canvasHeight);
        if (strokeWidth > 0) {
            canvas.setColor(strokeColor);
            canvas.drawRectangle(0, 0, width + strokeWidth * 0.5f, height + strokeWidth * 0.5f);
        }
        canvas.setColor(color);
        canvas.drawRectangle(strokeWidth, strokeWidth, width - strokeWidth * 0.5f, height - strokeWidth * 0.5f);
        return canvas.buildBitmap();
    }
} }
