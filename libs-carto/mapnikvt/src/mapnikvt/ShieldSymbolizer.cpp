#include "ShieldSymbolizer.h"

namespace carto { namespace mvt {
    void ShieldSymbolizer::build(const FeatureCollection& featureCollection, const SymbolizerContext& symbolizerContext, const ExpressionContext& exprContext, vt::TileLayerBuilder& layerBuilder) {
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

        float fontScale = symbolizerContext.getSettings().getFontScale();
        float bitmapSize = static_cast<float>(std::max(backgroundBitmap->width, backgroundBitmap->height)) * fontScale;
        std::string text = getTransformedText(_text);
        std::size_t hash = std::hash<std::string>()(text);
        vt::LabelOrientation placement = convertTextPlacement(_placement);
        float minimumDistance = (_minimumDistance + bitmapSize) * std::pow(2.0f, -exprContext.getZoom()) / symbolizerContext.getSettings().getTileSize() * 2;
        long long groupId = (_allowOverlap ? -1 : 1); // use separate group from markers, markers use group 0

        vt::TextFormatter::Options textFormatterOptions = getFormatterOptions(symbolizerContext);
        vt::TextFormatter::Options shieldFormatterOptions = textFormatterOptions;
        shieldFormatterOptions.offset = cglib::vec2<float>(_shieldDx * fontScale, -_shieldDy * fontScale);

        auto addLabel = [&](long long id, vt::LabelOrientation placement, const boost::optional<vt::TileLayerBuilder::Vertex>& position, const vt::TileLayerBuilder::Vertices& vertices) {
            if (_unlockImage) {
                cglib::vec2<float> backgroundOffset(-backgroundBitmap->width * fontScale * 0.5f + shieldFormatterOptions.offset(0), -backgroundBitmap->height * fontScale * 0.5f + shieldFormatterOptions.offset(1));
                vt::TextLabelStyle textStyle(placement, textFormatterOptions, font, _orientation, fontScale, backgroundOffset, backgroundBitmap);
                layerBuilder.addTextLabel(id, groupId, text, position, vertices, minimumDistance, textStyle);
            }
            else {
                cglib::vec2<float> backgroundOffset(-backgroundBitmap->width * fontScale * 0.5f, -backgroundBitmap->height * fontScale * 0.5f);
                vt::TextLabelStyle style(placement, shieldFormatterOptions, font, _orientation, fontScale, backgroundOffset, backgroundBitmap);
                layerBuilder.addTextLabel(id, groupId, text, position, vertices, minimumDistance, style);
            }
        };

        for (std::size_t index = 0; index < featureCollection.getSize(); index++) {
            long long featureId = featureCollection.getId(index);
            const std::shared_ptr<Geometry>& geometry = featureCollection.getGeometry(index);

            if (auto pointGeometry = std::dynamic_pointer_cast<const PointGeometry>(geometry)) {
                for (const auto& vertex : pointGeometry->getVertices()) {
                    long long id = getShieldId(featureId, hash);
                    addLabel(id, placement, vertex, vt::TileLayerBuilder::Vertices());
                }
            }
            else if (auto lineGeometry = std::dynamic_pointer_cast<const LineGeometry>(geometry)) {
                if (placement == vt::LabelOrientation::LINE) {
                    for (const auto& vertices : lineGeometry->getVerticesList()) {
                        if (_spacing <= 0) {
                            addLabel(getShieldId(featureId, hash), vt::LabelOrientation::BILLBOARD_2D, boost::optional<vt::TileLayerBuilder::Vertex>(), vertices);
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
                                    addLabel(getMultiShieldId(featureId, hash), vt::LabelOrientation::BILLBOARD_2D, pos, vertices);
                                }

                                linePos += _spacing + bitmapSize;
                            }

                            linePos -= lineLen;
                        }
                    }
                }
                else {
                    for (const auto& vertices : lineGeometry->getVerticesList()) {
                        long long id = getShieldId(featureId, hash);
                        addLabel(id, placement, boost::optional<vt::TileLayerBuilder::Vertex>(), vertices);
                    }
                }
            }
            else if (auto polygonGeometry = std::dynamic_pointer_cast<const PolygonGeometry>(geometry)) {
                for (const auto& vertex : polygonGeometry->getSurfacePoints()) {
                    long long id = getShieldId(featureId, hash);
                    addLabel(id, placement, vertex, vt::TileLayerBuilder::Vertices());
                }
            }
            else {
                _logger->write(Logger::Severity::WARNING, "Unsupported geometry for ShieldSymbolizer");
            }
        }
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
