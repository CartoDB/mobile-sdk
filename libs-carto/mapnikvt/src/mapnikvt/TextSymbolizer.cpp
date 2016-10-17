#include "TextSymbolizer.h"
#include "ParserUtils.h"
#include "FontSet.h"
#include "Expression.h"
#include "ExpressionOperator.h"
#include "vt/FontManager.h"

#include <memory>
#include <vector>
#include <limits>

#include <boost/algorithm/string.hpp>

namespace carto { namespace mvt {
    void TextSymbolizer::setTextExpression(std::shared_ptr<Expression> textExpression) {
        _textExpression = std::move(textExpression);
        bind(&_text, _textExpression);
    }

    const std::shared_ptr<Expression>& TextSymbolizer::getTextExpression() const {
        return _textExpression;
    }
    
    void TextSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        std::shared_ptr<vt::Font> font = getFont(symbolizerContext);
        if (!font) {
            _logger->write(Logger::Severity::ERROR, "Failed to load text font " + (!_faceName.empty() ? _faceName :_fontSetName));
            return;
        }

        vt::CompOp compOp = vt::CompOp::SRC_OVER;

        std::string text = getTransformedText(_text);
        std::size_t hash = std::hash<std::string>()(text);
        vt::TextFormatter::Options textFormatterOptions = getFormatterOptions(symbolizerContext);

        float fontScale = symbolizerContext.getSettings().getFontScale();
        vt::LabelOrientation placement = convertTextPlacement(_placement);
        float minimumDistance = _minimumDistance * std::pow(2.0f, -exprContext.getZoom());
        float textSize = (placement == vt::LabelOrientation::LINE ? calculateTextSize(font, text, textFormatterOptions).size()(0) : 0);
        long long groupId = (_allowOverlap ? -1 : (minimumDistance > 0 ? (hash & 0x7fffffff) : 0));

        std::vector<std::pair<long long, vt::TileLayerBuilder::Vertex>> textInfos;
        std::vector<std::pair<long long, vt::TileLayerBuilder::TextLabelInfo>> labelInfos;

        auto addText = [&](long long localId, long long globalId, const boost::optional<vt::TileLayerBuilder::Vertex>& vertex, const vt::TileLayerBuilder::Vertices& vertices) {
            if (_allowOverlap) {
                if (vertex) {
                    textInfos.emplace_back(localId, *vertex);
                }
                else if (!vertices.empty()) {
                    textInfos.emplace_back(localId, vertices.front());
                }
            }
            else {
                labelInfos.emplace_back(localId, vt::TileLayerBuilder::TextLabelInfo(getTextId(globalId, hash), groupId, text, vertex, vertices, minimumDistance));
            }
        };

        auto flushTexts = [&](const cglib::mat3x3<float>& transform) {
            if (_allowOverlap) {
                std::shared_ptr<const vt::ColorFunction> fillFunc;
                ExpressionFunctionBinder<vt::Color>().bind(&fillFunc, std::make_shared<ConstExpression>(Value(std::string("#ffffff"))), [this](const Value& val) -> vt::Color {
                    return convertColor(val);
                }).update(exprContext);
                std::shared_ptr<const vt::FloatFunction> opacityFunc;
                ExpressionFunctionBinder<float>().bind(&opacityFunc, std::make_shared<ConstExpression>(Value(1.0f))).update(exprContext);

                vt::TextStyle style(compOp, convertLabelToPointOrientation(placement), fillFunc, opacityFunc, textFormatterOptions, font, _orientation, fontScale, cglib::vec2<float>(0, 0), std::shared_ptr<vt::Bitmap>(), transform);

                std::size_t textInfoIndex = 0;
                layerBuilder.addTexts([&](long long& id, vt::TileLayerBuilder::Vertex& vertex, std::string& txt) {
                    if (textInfoIndex >= textInfos.size()) {
                        return false;
                    }
                    id = textInfos[textInfoIndex].first;
                    vertex = textInfos[textInfoIndex].second;
                    txt = text;
                    textInfoIndex++;
                    return true;
                }, style);

                textInfos.clear();
            }
            else {
                vt::TextLabelStyle style(placement, textFormatterOptions, font, _orientation, fontScale, cglib::vec2<float>(0, 0), std::shared_ptr<vt::Bitmap>());

                std::size_t labelInfoIndex = 0;
                layerBuilder.addTextLabels([&](long long& id, vt::TileLayerBuilder::TextLabelInfo& labelInfo) {
                    if (labelInfoIndex >= labelInfos.size()) {
                        return false;
                    }
                    id = labelInfos[labelInfoIndex].first;
                    labelInfo = std::move(labelInfos[labelInfoIndex].second);
                    labelInfoIndex++;
                    return true;
                }, style);

                labelInfos.clear();
            }
        };

        buildFeatureCollection(featureCollection, symbolizerContext, placement, textSize, addText);

        flushTexts(cglib::mat3x3<float>::identity());
    }

    void TextSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "name") {
            bind(&_text, std::make_shared<VariableExpression>(value));
        }
        else if (name == "face-name") {
            bind(&_faceName, parseStringExpression(value));
        }
        else if (name == "fontset-name") {
            bind(&_fontSetName, parseStringExpression(value));
        }
        else if (name == "placement") {
            bind(&_placement, parseStringExpression(value));
        }
        else if (name == "size") {
            bind(&_size, parseExpression(value));
        }
        else if (name == "spacing") {
            bind(&_spacing, parseExpression(value));
        }
        else if (name == "fill") {
            bind(&_fill, parseStringExpression(value), &TextSymbolizer::convertColor);
        }
        else if (name == "opacity") {
            bind(&_opacity, parseExpression(value));
        }
        else if (name == "halo-fill") {
            bind(&_haloFill, parseStringExpression(value), &TextSymbolizer::convertColor);
        }
        else if (name == "halo-opacity") {
            bind(&_haloOpacity, parseExpression(value));
        }
        else if (name == "halo-radius") {
            bind(&_haloRadius, parseExpression(value));
        }
        else if (name == "halo-rasterizer") {
            // just ignore this
        }
        else if (name == "allow-overlap") {
            bind(&_allowOverlap, parseExpression(value));
        }
        else if (name == "minimum-distance") {
            bind(&_minimumDistance, parseExpression(value));
        }
        else if (name == "text-transform") {
            bind(&_textTransform, parseStringExpression(value));
        }
        else if (name == "orientation") {
            bind(&_orientation, parseExpression(value));
            _orientationDefined = true;
        }
        else if (name == "dx") {
            bind(&_dx, parseExpression(value));
        }
        else if (name == "dy") {
            bind(&_dy, parseExpression(value));
        }
        else if (name == "avoid-edges") {
            // can ignore this, we are not clipping texts at tile boundaries
        }
        else if (name == "wrap-width") {
            bind(&_wrapWidth, parseExpression(value));
        }
        else if (name == "wrap-before") {
            bind(&_wrapBefore, parseExpression(value));
        }
        else if (name == "character-spacing") {
            bind(&_characterSpacing, parseExpression(value));
        }
        else if (name == "line-spacing") {
            bind(&_lineSpacing, parseExpression(value));
        }
        else if (name == "horizontal-alignment") {
            bind(&_horizontalAlignment, parseStringExpression(value));
        }
        else if (name == "vertical-alignment") {
            bind(&_verticalAlignment, parseStringExpression(value));
        }
        else {
            Symbolizer::bindParameter(name, value);
        }
    }

    std::string TextSymbolizer::getTransformedText(const std::string& text) const {
        if (_textTransform.empty()) {
            return text;
        }
        else if (_textTransform == "uppercase") {
            return toUpper(text);
        }
        else if (_textTransform == "lowercase") {
            return toLower(text);
        }
        else if (_textTransform == "capitalize") {
            return capitalize(text);
        }
        return text;
    }

    std::shared_ptr<vt::Font> TextSymbolizer::getFont(const SymbolizerContext& symbolizerContext) const {
        std::shared_ptr<vt::Font> font;
        float fontScale = symbolizerContext.getSettings().getFontScale();
        if (!_faceName.empty()) {
            vt::FontManager::Parameters fontParams(_size * fontScale, _fill * _opacity, _haloRadius * fontScale, _haloFill * _haloOpacity, std::shared_ptr<vt::Font>());
            font = symbolizerContext.getFontManager()->getFont(_faceName, fontParams);
        }
        else if (!_fontSetName.empty()) {
            for (const std::shared_ptr<FontSet>& fontSet : _fontSets) {
                if (fontSet->getName() == _fontSetName) {
                    const std::vector<std::string>& faceNames = fontSet->getFaceNames();
                    for (auto it = faceNames.rbegin(); it != faceNames.rend(); it++) {
                        const std::string& faceName = *it;
                        vt::FontManager::Parameters fontParams(_size * fontScale, _fill * _opacity, _haloRadius * fontScale, _haloFill * _haloOpacity, font);
                        std::shared_ptr<vt::Font> mainFont = symbolizerContext.getFontManager()->getFont(faceName, fontParams);
                        if (mainFont) {
                            font = mainFont;
                        }
                    }
                    break;
                }
            }
        }
        return font;
    }

    cglib::bbox2<float> TextSymbolizer::calculateTextSize(const std::shared_ptr<vt::Font>& font, const std::string& text, const vt::TextFormatter::Options& formatterOptions) const {
        vt::TextFormatter formatter(font);
        std::vector<vt::Font::Glyph> glyphs = formatter.format(text, formatterOptions);
        cglib::bbox2<float> bbox = cglib::bbox2<float>::smallest();
        cglib::vec2<float> pen = cglib::vec2<float>(0, 0);
        for (const vt::Font::Glyph& glyph : glyphs) {
            if (glyph.codePoint == vt::Font::CR_CODEPOINT) {
                pen = cglib::vec2<float>(0, 0);
            }
            else {
                bbox.add(pen + glyph.offset);
                bbox.add(pen + glyph.offset + glyph.size);
            }

            pen += glyph.advance;
        }
        return bbox;
    }

    vt::TextFormatter::Options TextSymbolizer::getFormatterOptions(const SymbolizerContext& symbolizerContext) const {
        float fontScale = symbolizerContext.getSettings().getFontScale();
        cglib::vec2<float> offset(_dx * fontScale, -_dy * fontScale);
        cglib::vec2<float> alignment(_dx < 0 ? 1.0f : (_dx > 0 ? -1.0f : 0.0f), _dy < 0 ? 1.0f : (_dy > 0 ? -1.0f : 0.0f));
        if (_horizontalAlignment == "left") {
            alignment(0) = -1.0f;
        }
        else if (_horizontalAlignment == "middle") {
            alignment(0) = 0.0f;
        }
        else if (_horizontalAlignment == "right") {
            alignment(0) = 1.0f;
        }
        if (_verticalAlignment == "top") {
            alignment(1) = -1.0f;
        }
        else if (_verticalAlignment == "middle") {
            alignment(1) = 0.0f;
        }
        else if (_verticalAlignment == "bottom") {
            alignment(1) = 1.0f;
        }
        return vt::TextFormatter::Options(alignment, offset, _wrapBefore, _wrapWidth * fontScale, _characterSpacing, _lineSpacing);
    }

    vt::LabelOrientation TextSymbolizer::convertTextPlacement(const std::string& orientation) const {
        vt::LabelOrientation placement = convertLabelPlacement(orientation);
        if (placement != vt::LabelOrientation::LINE) {
            if (_orientationDefined) { // if orientation is explictly defined, use POINT placement
                return vt::LabelOrientation::POINT_FLIPPING;
            }
            if (placement == vt::LabelOrientation::POINT) { // texts should flip if viewed from upside down
                return vt::LabelOrientation::POINT_FLIPPING;
            }
        }
        return placement;
    }

    void TextSymbolizer::buildFeatureCollection(const FeatureCollection& featureCollection, const SymbolizerContext& symbolizerContext, vt::LabelOrientation placement, float textSize, const std::function<void(long long localId, long long globalId, const boost::optional<vt::TileLayerBuilder::Vertex>& vertex, const vt::TileLayerBuilder::Vertices& vertices)>& addText) {
        for (std::size_t index = 0; index < featureCollection.getSize(); index++) {
            long long localId = featureCollection.getLocalId(index);
            long long globalId = featureCollection.getGlobalId(index);
            const std::shared_ptr<const Geometry>& geometry = featureCollection.getGeometry(index);

            if (auto pointGeometry = std::dynamic_pointer_cast<const PointGeometry>(geometry)) {
                for (const auto& vertex : pointGeometry->getVertices()) {
                    addText(localId, globalId, vertex, vt::TileLayerBuilder::Vertices());
                }
            }
            else if (auto lineGeometry = std::dynamic_pointer_cast<const LineGeometry>(geometry)) {
                if (placement == vt::LabelOrientation::LINE) {
                    for (const auto& vertices : lineGeometry->getVerticesList()) {
                        if (_spacing <= 0) {
                            addText(localId, globalId, boost::optional<vt::TileLayerBuilder::Vertex>(), vertices);
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
                                    addText(localId, 0, pos, vertices);
                                }

                                linePos += _spacing + textSize;
                            }

                            linePos -= lineLen;
                        }
                    }
                }
                else {
                    for (const auto& vertices : lineGeometry->getVerticesList()) {
                        addText(localId, globalId, boost::optional<vt::TileLayerBuilder::Vertex>(), vertices);
                    }
                }
            }
            else if (auto polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(geometry)) {
                for (const auto& vertex : polygonGeometry->getSurfacePoints()) {
                    addText(localId, globalId, vertex, vt::TileLayerBuilder::Vertices());
                }
            }
            else {
                _logger->write(Logger::Severity::WARNING, "Unsupported geometry for TextSymbolizer/ShieldSymbolizer");
            }
        }
    }
} }
