#include "TorqueCartoCSSMapLoader.h"
#include "CartoCSSParser.h"
#include "TorqueCartoCSSMapnikTranslator.h"
#include "mapnikvt/TorqueLayer.h"

namespace carto { namespace css {
    std::shared_ptr<mvt::TorqueMap> TorqueCartoCSSMapLoader::loadMap(const std::string& cartoCSS) const {
        StyleSheet styleSheet;
        try {
            styleSheet = CartoCSSParser::parse(cartoCSS);
        }
        catch (const CartoCSSParser::ParserError& ex) {
            std::string msg = std::string("Error while parsing Torque CartoCSS");
            if (ex.position().first != 0) {
                msg += ", error at line " + boost::lexical_cast<std::string>(ex.position().second) + ", column " + boost::lexical_cast<std::string>(ex.position().first);
            }
            msg += std::string(": ") + ex.what();
            throw LoaderException(msg);
        }
        catch (const std::exception& ex) {
            throw LoaderException(std::string("Exception while parsing Torque CartoCSS: ") + ex.what());
        }

        // Find layer names and frame offsets
        std::set<std::string> layerNames;
        std::set<int> frameOffsets;
        frameOffsets.insert(0);
        std::function<void(const RuleSet& ruleSet)> storeRuleSetInfo;
        storeRuleSetInfo = [&](const RuleSet& ruleSet) {
            for (const Selector& selector : ruleSet.getSelectors()) {
                for (const std::shared_ptr<const Predicate>& pred : selector.getPredicates()) {
                    if (auto layerPred = std::dynamic_pointer_cast<const LayerPredicate>(pred)) {
                        layerNames.insert(layerPred->getLayerName());
                    }
                    else if (auto opPred = std::dynamic_pointer_cast<const OpPredicate>(pred)) {
                        if (opPred->isField() && opPred->getFieldOrVar() == "frame-offset") {
                            if (opPred->getOp() == OpPredicate::Op::EQ) {
                                if (auto integer = boost::get<long long>(opPred->getRefValue())) {
                                    frameOffsets.insert(static_cast<int>(integer));
                                }
                                else {
                                    _logger->write(mvt::Logger::Severity::ERROR, "Torque 'frame-offset' value must be integer");
                                }
                            }
                            else {
                                _logger->write(mvt::Logger::Severity::ERROR, "Torque 'frame-offset' can be only used with equal operator");
                            }
                        }
                    }
                }
            }

            for (const Block::Element& element : ruleSet.getBlock().getElements()) {
                if (auto subRuleSet = boost::get<RuleSet>(&element)) {
                    storeRuleSetInfo(*subRuleSet);
                }
            }
        };

        for (const StyleSheet::Element& element : styleSheet.getElements()) {
            if (auto ruleSet = boost::get<RuleSet>(&element)) {
                storeRuleSetInfo(*ruleSet);
            }
        }

        // Map/torque properties
        mvt::TorqueMap::Settings mapSettings;
        mvt::TorqueMap::TorqueSettings torqueSettings;
        {
            try {
                CartoCSSCompiler compiler;
                std::map<std::string, Value> mapProperties;
                compiler.compileMap(styleSheet, mapProperties);

                loadMapSettings(mapProperties, mapSettings);
                loadTorqueSettings(mapProperties, torqueSettings);
            }
            catch (const std::exception& ex) {
                throw LoaderException(std::string("Error while building/loading map properties: ") + ex.what());
            }
        }
        auto map = std::make_shared<mvt::TorqueMap>(mapSettings, torqueSettings);

        // Layers
        CartoCSSCompiler compiler;
        TorqueCartoCSSMapnikTranslator translator(_logger);
        for (const std::string& layerName : layerNames) {
            for (auto frameOffsetIt = frameOffsets.rbegin(); frameOffsetIt != frameOffsets.rend(); frameOffsetIt++) {
                int frameOffset = *frameOffsetIt;
                std::map<std::string, AttachmentStyle> attachmentStyleMap;
                int minZoom = 0;
                std::list<CartoCSSCompiler::LayerAttachment> prevLayerAttachments;
                for (int zoom = 0; zoom < MAX_ZOOM; zoom++) {
                    try {
                        ExpressionContext context;
                        std::map<std::string, Value> predefinedFieldMap;
                        context.predefinedFieldMap = &predefinedFieldMap;
                        (*context.predefinedFieldMap)["zoom"] = Value(static_cast<long long>(zoom));
                        (*context.predefinedFieldMap)["frame-offset"] = Value(static_cast<long long>(frameOffset));
                        compiler.setContext(context);
                        compiler.setIgnoreLayerPredicates(_ignoreLayerPredicates);

                        std::list<CartoCSSCompiler::LayerAttachment> layerAttachments;
                        compiler.compileLayer(layerName, styleSheet, layerAttachments);

                        if (zoom > 0 && layerAttachments != prevLayerAttachments) {
                            buildAttachmentStyleMap(translator, map, minZoom, zoom, prevLayerAttachments, attachmentStyleMap);
                            minZoom = zoom;
                        }
                        prevLayerAttachments = std::move(layerAttachments);
                    }
                    catch (const std::exception& ex) {
                        throw LoaderException(std::string("Error while building zoom ") + boost::lexical_cast<std::string>(zoom) + " properties: " + ex.what());
                    }
                }
                buildAttachmentStyleMap(translator, map, minZoom, MAX_ZOOM, prevLayerAttachments, attachmentStyleMap);

                if (attachmentStyleMap.empty()) {
                    continue;
                }
                std::vector<AttachmentStyle> attachmentStyles = getSortedAttachmentStyles(attachmentStyleMap);

                std::vector<std::string> styleNames;
                for (const AttachmentStyle& attachmentStyle : attachmentStyles) {
                    // Build style, but ignore layer level opacity and comp-op
                    std::string styleName = layerName + attachmentStyle.attachment + "#" + boost::lexical_cast<std::string>(frameOffset);
                    auto style = std::make_shared<mvt::Style>(styleName, 1.0f, std::string(), mvt::Style::FilterMode::FIRST, attachmentStyle.rules);
                    map->addStyle(style);
                    styleNames.push_back(styleName);
                }
                auto layer = std::make_shared<mvt::TorqueLayer>(layerName, frameOffset, styleNames);
                map->addLayer(layer);
            }
        }
        return map;
    }

    void TorqueCartoCSSMapLoader::loadTorqueSettings(const std::map<std::string, Value>& mapProperties, mvt::TorqueMap::TorqueSettings& torqueSettings) const {
        Color clearColor;
        if (getMapProperty(mapProperties, "-torque-clear-color", clearColor)) {
            torqueSettings.clearColor = vt::Color(clearColor.value());
        }
        long long frameCount = 0;
        if (getMapProperty(mapProperties, "-torque-frame-count", frameCount)) {
            torqueSettings.frameCount = static_cast<int>(frameCount);
        }
        long long animationDuration = 0;
        if (getMapProperty(mapProperties, "-torque-animation-duration", animationDuration)) {
            torqueSettings.animationDuration = static_cast<int>(animationDuration);
        }
        long long resolution = 1;
        if (getMapProperty(mapProperties, "-torque-resolution", resolution)) {
            torqueSettings.resolution = static_cast<int>(resolution);
        }
        getMapProperty(mapProperties, "-torque-time-attribute", torqueSettings.timeAttribute);
        getMapProperty(mapProperties, "-torque-aggregation-function", torqueSettings.aggregationFunction);
        getMapProperty(mapProperties, "-torque-data-aggregation", torqueSettings.dataAggregation);
    }
} }
