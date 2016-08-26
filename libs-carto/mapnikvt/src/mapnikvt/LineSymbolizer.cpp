#include "LineSymbolizer.h"
#include "ParserUtils.h"
#include "vt/BitmapCanvas.h"

#include <cmath>

#include <boost/algorithm/string.hpp>

namespace carto { namespace mvt {
    void LineSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        vt::LineJoinMode lineJoin = convertLineJoinMode(_strokeLinejoin);
        vt::LineCapMode lineCap = convertLineCapMode(_strokeLinecap);
        vt::CompOp compOp = convertCompOp(_compOp);
        
        std::shared_ptr<const vt::BitmapPattern> strokePattern;
        if (!_strokeDashArray.empty()) {
            std::string file = "__line_dasharray_" + _strokeDashArray;
            strokePattern = symbolizerContext.getBitmapManager()->getBitmapPattern(file);
            if (!strokePattern) {
                std::vector<std::string> dashList;
                boost::split(dashList, _strokeDashArray, boost::is_any_of(","));
                std::vector<float> strokeDashArray;
                for (const std::string& dash : dashList) {
                    try {
                        strokeDashArray.push_back(boost::lexical_cast<float>(boost::trim_copy(dash)));
                    }
                    catch (const boost::bad_lexical_cast&) {
                        _logger->write(Logger::Severity::ERROR, "Illegal dash value");
                    }
                }
                if (strokeDashArray.empty()) {
                    strokeDashArray.push_back(1);
                }
                strokePattern = createDashBitmapPattern(strokeDashArray);
                symbolizerContext.getBitmapManager()->storeBitmapPattern(file, strokePattern);
            }
        }
        
        vt::LineStyle style(compOp, lineJoin, lineCap, _stroke, _strokeOpacity, _strokeWidth, symbolizerContext.getStrokeMap(), strokePattern, _geometryTransform);

        std::size_t featureIndex = 0;
        std::size_t geometryIndex = 0;
        std::size_t polygonIndex = 0;
        std::shared_ptr<const LineGeometry> lineGeometry;
        std::shared_ptr<const PolygonGeometry> polygonGeometry;
        layerBuilder.addLines([&](long long& id, vt::TileLayerBuilder::Vertices& vertices) {
            while (true) {
                if (lineGeometry) {
                    if (geometryIndex < lineGeometry->getVerticesList().size()) {
                        vertices = lineGeometry->getVerticesList()[geometryIndex++];
                        return true;
                    }
                }
                if (polygonGeometry) {
                    while (geometryIndex < polygonGeometry->getPolygonList().size()) {
                        if (polygonIndex < polygonGeometry->getPolygonList()[geometryIndex].size()) {
                            vertices = polygonGeometry->getPolygonList()[geometryIndex][polygonIndex++];
                            return true;
                        }
                        geometryIndex++;
                        polygonIndex = 0;
                    }
                }

                if (featureIndex >= featureCollection.getSize()) {
                    break;
                }
                lineGeometry = std::dynamic_pointer_cast<const LineGeometry>(featureCollection.getGeometry(featureIndex));
                polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(featureCollection.getGeometry(featureIndex));
                if (!lineGeometry && !polygonGeometry) {
                    _logger->write(Logger::Severity::WARNING, "Unsupported geometry for LineSymbolizer");
                }
                featureIndex++;
                geometryIndex = 0;
            }
            return false;
        }, style);
    }

    vt::LineCapMode LineSymbolizer::convertLineCapMode(const std::string& lineCap) const {
        if (lineCap == "round") {
            return vt::LineCapMode::ROUND;
        }
        else if (lineCap == "square") {
            return vt::LineCapMode::SQUARE;
        }
        else if (lineCap == "butt") {
            return vt::LineCapMode::NONE;
        }
        _logger->write(Logger::Severity::ERROR, std::string("Unsupported line cap mode: ") + lineCap);
        return vt::LineCapMode::NONE;
    }

    vt::LineJoinMode LineSymbolizer::convertLineJoinMode(const std::string& lineJoin) const {
        if (_strokeLinejoin == "round") {
            return vt::LineJoinMode::ROUND;
        }
        else if (_strokeLinejoin == "bevel") {
            return vt::LineJoinMode::BEVEL;
        }
        else if (_strokeLinejoin == "miter") {
            return vt::LineJoinMode::MITER;
        }
        _logger->write(Logger::Severity::ERROR, std::string("Unsupported line join mode: ") + lineJoin);
        return vt::LineJoinMode::MITER;
    }

    void LineSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "stroke") {
            bind(&_stroke, parseStringExpression(value), &LineSymbolizer::convertColor);
        }
        else if (name == "stroke-width") {
            bind(&_strokeWidth, parseExpression(value));
        }
        else if (name == "stroke-opacity") {
            bind(&_strokeOpacity, parseExpression(value));
        }
        else if (name == "stroke-linejoin") {
            bind(&_strokeLinejoin, parseStringExpression(value));
        }
        else if (name == "stroke-linecap") {
            bind(&_strokeLinecap, parseStringExpression(value));
        }
        else if (name == "stroke-dasharray") {
            bind(&_strokeDashArray, parseStringExpression(value));
        }
        else {
            GeometrySymbolizer::bindParameter(name, value);
        }
    }

    std::shared_ptr<vt::BitmapPattern> LineSymbolizer::createDashBitmapPattern(const std::vector<float>& strokeDashArray) {
        float size = 0;
        int superSample = MIN_SUPERSAMPLING_FACTOR;
        for (float dash : strokeDashArray) {
            size += dash;
            int factor = 1;
            while (factor * superSample < MAX_SUPERSAMPLING_FACTOR) { // increase resolution when fractional dashes are used
                float dashFract = dash * superSample * factor;
                dashFract -= std::floor(dashFract);
                if (dashFract < 0.1f || dashFract > 0.9f) {
                    break;
                }
                factor++;
            }
            superSample *= factor;
        }
        int pow2Size = 1;
        while (pow2Size < size * superSample && pow2Size < 2048) {
            pow2Size *= 2;
        }

        vt::BitmapCanvas canvas(pow2Size, 1);
        float pos = 0;
        for (std::size_t n = 0; n < strokeDashArray.size(); n++) {
            float dash = strokeDashArray[n];
            if (n % 2 == 0) {
                float x0 = pos * pow2Size / size;
                float x1 = (pos + dash) * pow2Size / size;
                canvas.drawRectangle(x0, 0, x1, 1);
            }
            pos += dash;
        }
        return std::make_shared<vt::BitmapPattern>(0.75f * size / pow2Size, 1.0f, canvas.buildBitmap());
    }
} }
