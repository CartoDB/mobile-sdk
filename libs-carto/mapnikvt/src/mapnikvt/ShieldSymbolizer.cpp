#include "ShieldSymbolizer.h"

namespace carto { namespace mvt {
    void ShieldSymbolizer::build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) {
        std::lock_guard<std::mutex> lock(_mutex);

        updateBindings(exprContext);

        std::shared_ptr<vt::Font> font = getFont(symbolizerContext);
        if (!font) {
            _logger->write(Logger::Severity::ERROR, "Failed to load shield font " + (!_faceName.empty() ? _faceName : _fontSetName));
            return;
        }

        std::shared_ptr<const vt::Bitmap> backgroundBitmap = symbolizerContext.getBitmapManager()->loadBitmap(_file);
        if (!backgroundBitmap) {
            _logger->write(Logger::Severity::ERROR, "Failed to load shield bitmap " + _file);
            return;
        }

        vt::CompOp compOp = vt::CompOp::SRC_OVER;

        float fontScale = symbolizerContext.getSettings().getFontScale();
        float bitmapSize = static_cast<float>(std::max(backgroundBitmap->width, backgroundBitmap->height)) * fontScale;
        vt::LabelOrientation placement = convertTextPlacement(_placement);
        vt::LabelOrientation orientation = placement;
        if (orientation == vt::LabelOrientation::LINE) {
            orientation = vt::LabelOrientation::BILLBOARD_2D; // shields should be billboards, even when placed on a line
        }

        std::string text = getTransformedText(_text);
        std::size_t hash = std::hash<std::string>()(text);
        vt::TextFormatter::Options textFormatterOptions = getFormatterOptions(symbolizerContext);
        vt::TextFormatter::Options shieldFormatterOptions = textFormatterOptions;
        shieldFormatterOptions.offset = cglib::vec2<float>(_shieldDx * fontScale, -_shieldDy * fontScale);

        float minimumDistance = (_minimumDistance + bitmapSize) * std::pow(2.0f, -exprContext.getZoom()) / symbolizerContext.getSettings().getTileSize() * 2;
        long long groupId = (_allowOverlap ? -1 : 1); // use separate group from markers, markers use group 0

        std::vector<std::pair<long long, vt::TileLayerBuilder::Vertex>> shieldInfos;
        std::vector<std::pair<long long, vt::TileLayerBuilder::TextLabelInfo>> labelInfos;

        auto addShield = [&](long long localId, long long globalId, const boost::optional<vt::TileLayerBuilder::Vertex>& vertex, const vt::TileLayerBuilder::Vertices& vertices) {
            if (_allowOverlap) {
                if (vertex) {
                    shieldInfos.emplace_back(localId, *vertex);
                }
                else if (!vertices.empty()) {
                    shieldInfos.emplace_back(localId, vertices.front());
                }
            }
            else {
                labelInfos.emplace_back(localId, vt::TileLayerBuilder::TextLabelInfo(getShieldId(globalId, hash), groupId, text, vertex, vertices, minimumDistance));
            }
        };

        auto flushShields = [&](const cglib::mat3x3<float>& transform) {
            cglib::vec2<float> backgroundOffset;
            const vt::TextFormatter::Options* formatterOptions;
            if (_unlockImage) {
                backgroundOffset = cglib::vec2<float>(-backgroundBitmap->width * fontScale * 0.5f + shieldFormatterOptions.offset(0), -backgroundBitmap->height * fontScale * 0.5f + shieldFormatterOptions.offset(1));
                formatterOptions = &textFormatterOptions;
            }
            else {
                backgroundOffset = cglib::vec2<float>(-backgroundBitmap->width * fontScale * 0.5f, -backgroundBitmap->height * fontScale * 0.5f);
                formatterOptions = &shieldFormatterOptions;
            }

            if (_allowOverlap) {
                std::shared_ptr<const vt::ColorFunction> fillFunc;
                ExpressionFunctionBinder<vt::Color>().bind(&fillFunc, std::make_shared<ConstExpression>(Value(std::string("#ffffff"))), [this](const Value& val) -> vt::Color {
                    return convertColor(val);
                }).update(exprContext);
                std::shared_ptr<const vt::FloatFunction> opacityFunc;
                ExpressionFunctionBinder<float>().bind(&opacityFunc, std::make_shared<ConstExpression>(Value(1.0f))).update(exprContext);

                vt::TextStyle style(compOp, convertLabelToPointOrientation(orientation), fillFunc, opacityFunc, *formatterOptions, font, _orientation, fontScale, backgroundOffset, backgroundBitmap, transform);

                std::size_t textInfoIndex = 0;
                layerBuilder.addTexts([&](long long& id, vt::TileLayerBuilder::Vertex& vertex, std::string& txt) {
                    if (textInfoIndex >= shieldInfos.size()) {
                        return false;
                    }
                    id = shieldInfos[textInfoIndex].first;
                    vertex = shieldInfos[textInfoIndex].second;
                    txt = text;
                    textInfoIndex++;
                    return true;
                }, style);

                shieldInfos.clear();
            }
            else {
                vt::TextLabelStyle style(placement, *formatterOptions, font, _orientation, fontScale, backgroundOffset, backgroundBitmap);

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

        buildFeatureCollection(featureCollection, symbolizerContext, placement, bitmapSize, addShield);

        flushShields(cglib::mat3x3<float>::identity());
    }

    void ShieldSymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "file") {
            bind(&_file, parseStringExpression(value));
        }
        else if (name == "shield-dx") {
            bind(&_shieldDx, parseExpression(value));
        }
        else if (name == "shield-dy") {
            bind(&_shieldDy, parseExpression(value));
        }
        else if (name == "unlock-image") {
            bind(&_unlockImage, parseExpression(value));
        }
        else {
            TextSymbolizer::bindParameter(name, value);
        }
    }
} }
