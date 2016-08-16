#include "MarkersSymbolizer.h"
#include "ParserUtils.h"
#include "vt/BitmapCanvas.h"

#include <boost/lexical_cast.hpp>

namespace carto { namespace mvt {
    void MarkersSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        vt::CompOp compOp = convertCompOp(_compOp);

        float fontScale = symbolizerContext.getSettings().getFontScale();
        vt::LabelOrientation placement = convertLabelPlacement(_placement);
        if (!_placementDefined) {
            if (_transformExpression) { // if rotation transform is explicitly defined, use point placement
                if (containsRotationTransform(_transformExpression->evaluate(exprContext))) {
                    placement = vt::LabelOrientation::POINT;
                }
            }
            if (_allowOverlap) { // if overlap is allowed, use point placement
                placement = vt::LabelOrientation::POINT;
            }
        }

        float bitmapScaleX = fontScale, bitmapScaleY = fontScale;
        std::shared_ptr<const vt::Bitmap> bitmap;
        std::string file = _file;
        float fillOpacity = _fillOpacity;
        if (!file.empty()) {
            bitmap = symbolizerContext.getBitmapManager()->loadBitmap(file);
            if (!bitmap) {
                _logger->write(Logger::Severity::ERROR, "Failed to load marker bitmap " + file);
                return;
            }
            if (_width > 0) {
                bitmapScaleX = fontScale * _width / bitmap->width;
                bitmapScaleY = (_height > 0 ? fontScale * _height / bitmap->height : bitmapScaleX);
            }
            else if (_height > 0) {
                bitmapScaleX = bitmapScaleY = fontScale * _height / bitmap->height;
            }
        }
        else {
            vt::Color fill = _fill * _fillOpacity;
            vt::Color stroke = _stroke * _strokeOpacity;
            if (_markerType == "ellipse" || (_markerType.empty() && placement != vt::LabelOrientation::LINE)) {
                float width = DEFAULT_CIRCLE_SIZE, height = DEFAULT_CIRCLE_SIZE;
                if (_width > 0) {
                    width = _width;
                    height = (_height > 0 ? _height : width);
                }
                else if (_height > 0) {
                    height = _height;
                    width = height;
                }
                file = "__default_marker_ellipse_" + boost::lexical_cast<std::string>(width) + "_" + boost::lexical_cast<std::string>(height) + "_" + boost::lexical_cast<std::string>(fill.value()) + "_" + boost::lexical_cast<std::string>(_strokeWidth) + "_" + boost::lexical_cast<std::string>(stroke.value()) + ".bmp";
                bitmap = symbolizerContext.getBitmapManager()->getBitmap(file);
                if (!bitmap) {
                    bitmap = makeEllipseBitmap(width * SUPERSAMPLING_FACTOR, height * SUPERSAMPLING_FACTOR, fill, std::abs(_strokeWidth) * SUPERSAMPLING_FACTOR, stroke);
                    symbolizerContext.getBitmapManager()->storeBitmap(file, bitmap);
                }
                bitmapScaleX = width  * fontScale / bitmap->width;
                bitmapScaleY = height * fontScale / bitmap->height;
            }
            else {
                float width = DEFAULT_ARROW_WIDTH, height = DEFAULT_ARROW_HEIGHT;
                if (_width > 0) {
                    width = _width;
                    height = (_height > 0 ? _height : DEFAULT_ARROW_HEIGHT * width / DEFAULT_ARROW_WIDTH);
                }
                else if (_height > 0) {
                    height = _height;
                    width = DEFAULT_ARROW_WIDTH * height / DEFAULT_ARROW_HEIGHT;
                }
                file = "__default_marker_arrow_" + boost::lexical_cast<std::string>(width) + "_" + boost::lexical_cast<std::string>(height) + "_" + boost::lexical_cast<std::string>(fill.value()) + "_" + boost::lexical_cast<std::string>(_strokeWidth) + "_" + boost::lexical_cast<std::string>(stroke.value()) + ".bmp";
                bitmap = symbolizerContext.getBitmapManager()->getBitmap(file);
                if (!bitmap) {
                    bitmap = makeArrowBitmap(width * SUPERSAMPLING_FACTOR, height * SUPERSAMPLING_FACTOR, fill, std::abs(_strokeWidth) * SUPERSAMPLING_FACTOR, stroke);
                    symbolizerContext.getBitmapManager()->storeBitmap(file, bitmap);
                }
                bitmapScaleX = width  * fontScale / bitmap->width;
                bitmapScaleY = height * fontScale / bitmap->height;
            }
            fillOpacity = 1.0f;
        }

        float bitmapSize = static_cast<float>(std::max(bitmap->width * bitmapScaleX, bitmap->height * bitmapScaleY));
        vt::BitmapLabelStyle style(placement, vt::Color(0xffffffff) * fillOpacity, symbolizerContext.getFontManager()->getNullFont(), bitmap, _transform * cglib::scale3_matrix(cglib::vec3<float>(bitmapScaleX, bitmapScaleY, 1)));
        int groupId = (_allowOverlap ? -1 : 0);

        std::unique_ptr<vt::PointStyle> pointStyle;
        if (_allowOverlap && placement == vt::LabelOrientation::POINT) {
            std::shared_ptr<const vt::FloatFunction> widthFunc;
            ExpressionFunctionBinder<float>().bind(&widthFunc, std::make_shared<ConstExpression>(Value(_width))).update(exprContext);
            std::shared_ptr<const vt::ColorFunction> fillFunc;
            ExpressionFunctionBinder<vt::Color>().bind(&fillFunc, std::make_shared<ConstExpression>(Value(std::string("#ffffff"))), [this](const Value& val) -> vt::Color {
                return convertColor(val);
            }).update(exprContext);
            std::shared_ptr<const vt::FloatFunction> opacityFunc;
            ExpressionFunctionBinder<float>().bind(&opacityFunc, std::make_shared<ConstExpression>(Value(fillOpacity))).update(exprContext);

            pointStyle = std::unique_ptr<vt::PointStyle>(new vt::PointStyle(compOp, fillFunc, opacityFunc, widthFunc, symbolizerContext.getGlyphMap(), bitmap, _transform));
        }

        auto addPoints = [&](long long id, const std::vector<cglib::vec2<float>>& vertices) {
            if (pointStyle) {
                layerBuilder.addPoints(vertices, *pointStyle);
            }
            else {
                for (const auto& vertex : vertices) {
                    layerBuilder.addBitmapLabel(id, groupId, vertex, 0, style);
                }
            }
        };

        for (std::size_t index = 0; index < featureCollection.getSize(); index++) {
            long long featureId = featureCollection.getId(index);
            const std::shared_ptr<const Geometry>& geometry = featureCollection.getGeometry(index);
            
            if (auto pointGeometry = std::dynamic_pointer_cast<const PointGeometry>(geometry)) {
                long long id = getBitmapId(featureId, file);
                addPoints(id, pointGeometry->getVertices());
            }
            else if (auto lineGeometry = std::dynamic_pointer_cast<const LineGeometry>(geometry)) {
                if (placement == vt::LabelOrientation::LINE) {
                    for (const auto& vertices : lineGeometry->getVerticesList()) {
                        if (_spacing <= 0) {
                            long long id = getBitmapId(featureId, file);
                            layerBuilder.addBitmapLabel(id, groupId, vertices, 0, style);
                            continue;
                        }

                        float linePos = 0;
                        for (std::size_t i = 1; i < vertices.size(); i++) {
                            const cglib::vec2<float>& v0 = vertices[i - 1];
                            const cglib::vec2<float>& v1 = vertices[i];

                            float lineLen = cglib::length(v1 - v0) * symbolizerContext.getSettings().getTileSize();
                            if (i == 1) {
                                linePos = std::min(lineLen, _spacing) * 0.5f;
                            }
                            while (linePos < lineLen) {
                                cglib::vec2<float> pos = v0 + (v1 - v0) * (linePos / lineLen);
                                if (std::min(pos(0), pos(1)) > 0.0f && std::max(pos(0), pos(1)) < 1.0f) {
                                    cglib::vec2<float> dir = cglib::unit(v1 - v0);
                                    cglib::mat3x3<float> dirTransform = cglib::mat3x3<float>::identity();
                                    dirTransform(0, 0) = dir(0);
                                    dirTransform(0, 1) = -dir(1);
                                    dirTransform(1, 0) = dir(1);
                                    dirTransform(1, 1) = dir(0);

                                    style.placement = vt::LabelOrientation::POINT;
                                    style.transform = dirTransform * _transform * cglib::scale3_matrix(cglib::vec3<float>(bitmapScaleX, bitmapScaleY, 1));

                                    long long id = getMultiBitmapId(featureId, file);
                                    layerBuilder.addBitmapLabel(id, groupId, pos, 0, style);
                                }

                                linePos += _spacing + bitmapSize;
                            }

                            linePos -= lineLen;
                        }
                    }
                }
                else {
                    long long id = getBitmapId(featureId, file);
                    addPoints(id, lineGeometry->getMidPoints());
                }
            }
            else if (auto polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(geometry)) {
                long long id = getBitmapId(featureId, file);
                addPoints(id, polygonGeometry->getSurfacePoints());
            }
            else {
                _logger->write(Logger::Severity::WARNING, "Unsupported geometry for MarkersSymbolizer");
            }
        }
    }

    void MarkersSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "file") {
            bind(&_file, parseStringExpression(value));
        }
        else if (name == "placement") {
            bind(&_placement, parseStringExpression(value));
            _placementDefined = true;
        }
        else if (name == "marker-type") {
            bind(&_markerType, parseStringExpression(value));
        }
        else if (name == "fill") {
            bind(&_fill, parseStringExpression(value), &MarkersSymbolizer::convertColor);
        }
        else if (name == "fill-opacity") {
            bind(&_fillOpacity, parseExpression(value));
        }
        else if (name == "width") {
            bind(&_width, parseExpression(value));
        }
        else if (name == "height") {
            bind(&_height, parseExpression(value));
        }
        else if (name == "stroke") {
            bind(&_stroke, parseStringExpression(value), &MarkersSymbolizer::convertColor);
        }
        else if (name == "stroke-opacity") {
            bind(&_strokeOpacity, parseExpression(value));
        }
        else if (name == "stroke-width") {
            bind(&_strokeWidth, parseExpression(value));
        }
        else if (name == "spacing") {
            bind(&_spacing, parseExpression(value));
        }
        else if (name == "allow-overlap") {
            bind(&_allowOverlap, parseExpression(value));
        }
        else if (name == "ignore-placement") {
            bind(&_ignorePlacement, parseExpression(value));
        }
        else if (name == "transform") {
            _transformExpression = parseStringExpression(value);
            bind(&_transform, _transformExpression, &MarkersSymbolizer::convertTransform);
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
    
    bool MarkersSymbolizer::containsRotationTransform(const Value& val) {
        try {
            std::vector<std::shared_ptr<Transform>> transforms = parseTransformList(boost::lexical_cast<std::string>(val));
            for (const std::shared_ptr<Transform>& transform : transforms) {
                if (std::dynamic_pointer_cast<RotateTransform>(transform)) {
                    return true;
                }
            }
            return false;
        }
        catch (const ParserException&) {
            return false;
        }
    }

    std::shared_ptr<vt::Bitmap> MarkersSymbolizer::makeEllipseBitmap(float width, float height, const vt::Color& color, float strokeWidth, const vt::Color& strokeColor) {
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

    std::shared_ptr<vt::Bitmap> MarkersSymbolizer::makeArrowBitmap(float width, float height, const vt::Color& color, float strokeWidth, const vt::Color& strokeColor) {
        int canvasWidth = static_cast<int>(std::ceil(width + strokeWidth));
        int canvasHeight = static_cast<int>(std::ceil(height + strokeWidth));
        float x0 = strokeWidth * 0.5f, x1 = std::ceil(width - height * 0.5f), y1 = height * 1 / 3, y2 = height * 2 / 3;
        vt::BitmapCanvas canvas(canvasWidth, canvasHeight);
        if (strokeWidth > 0) {
            canvas.setColor(strokeColor);
            canvas.drawRectangle(0, y1 - strokeWidth * 0.5f, x1, y2 + strokeWidth * 0.5f);
            canvas.drawTriangle(x1 - strokeWidth * 0.5f, 0, x1 - strokeWidth * 0.5f, height, width, height * 0.5f);
        }
        canvas.setColor(color);
        canvas.drawRectangle(x0, y1, x1, y2);
        canvas.drawTriangle(x1, strokeWidth, x1, height - strokeWidth * 0.5f, width - strokeWidth * 0.5f, height * 0.5f);
        return canvas.buildBitmap();
    }
} }
