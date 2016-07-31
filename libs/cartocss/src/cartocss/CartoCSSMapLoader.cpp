#include "CartoCSSMapLoader.h"
#include "CartoCSSParser.h"
#include "CartoCSSMapnikTranslator.h"

#include <picojson/picojson.h>

namespace carto { namespace css {
    namespace {
        mvt::Value convertJSONValue(const picojson::value& value) {
            if (value.is<std::string>()) {
                return mvt::Value(std::string(value.get<std::string>()));
            }
            if (value.is<bool>()) {
                return mvt::Value(value.get<bool>());
            }
            if (value.is<std::int64_t>()) {
                return mvt::Value(static_cast<long long>(value.get<std::int64_t>()));
            }
            if (value.is<double>()) {
                return mvt::Value(value.get<double>());
            }
            return mvt::Value();
        }
    }

    std::shared_ptr<mvt::Map> CartoCSSMapLoader::loadMap(const std::string& cartoCSS) const {
        StyleSheet styleSheet;
        try {
            styleSheet = CartoCSSParser::parse(cartoCSS);
        }
        catch (const CartoCSSParser::ParserError& ex) {
            std::string msg = std::string("Error while parsing CartoCSS");
            if (ex.position().first != 0) {
                msg += ", error at line " + boost::lexical_cast<std::string>(ex.position().second) + ", column " + boost::lexical_cast<std::string>(ex.position().first);
            }
            msg += std::string(": ") + ex.what();
            throw LoaderException(msg);
        }
        catch (const std::exception& ex) {
            throw LoaderException(std::string("Exception while parsing CartoCSS: ") + ex.what());
        }

        // Find layer names
        std::vector<std::string> layerNames;
        std::function<void(const RuleSet& ruleSet)> storeRuleSetInfo;
        storeRuleSetInfo = [&](const RuleSet& ruleSet) {
            for (const Selector& selector : ruleSet.getSelectors()) {
                for (const std::shared_ptr<const Predicate>& pred : selector.getPredicates()) {
                    if (auto layerPred = std::dynamic_pointer_cast<const LayerPredicate>(pred)) {
                        if (std::find(layerNames.begin(), layerNames.end(), layerPred->getLayerName()) == layerNames.end()) {
                            layerNames.push_back(layerPred->getLayerName());
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

        return buildMap(styleSheet, layerNames, std::vector<mvt::NutiParameter>());
    }

    std::shared_ptr<mvt::Map> CartoCSSMapLoader::loadMapProject(const std::string& fileName) const {
        std::shared_ptr<std::vector<unsigned char>> mapData = _assetLoader->load(fileName);
        if (!mapData) {
            throw LoaderException(std::string("Could not load map description file ") + fileName);
        }
        
        std::string mapJson(reinterpret_cast<const char*>(mapData->data()), reinterpret_cast<const char*>(mapData->data() + mapData->size()));
        picojson::value mapDoc;
        std::string err = picojson::parse(mapDoc, mapJson);
        if (!err.empty()) {
            throw LoaderException(std::string("Error while parsing map description: ") + err);
        }

        std::vector<std::string> mssFileNames;
        if (mapDoc.contains("styles")) {
            const picojson::value::array& stylesArr = mapDoc.get("styles").get<picojson::value::array>();
            for (auto jit = stylesArr.begin(); jit != stylesArr.end(); jit++) {
                mssFileNames.push_back(jit->get<std::string>());
            }
        }

        std::vector<std::string> layerNames;
        if (mapDoc.contains("layers")) {
            const picojson::value::array& layersArr = mapDoc.get("layers").get<picojson::value::array>();
            for (auto jit = layersArr.begin(); jit != layersArr.end(); jit++) {
                layerNames.insert(layerNames.begin(), jit->get<std::string>());
            }
        }

        // Combine stylesheet from individual fragments
        StyleSheet styleSheet;
        for (const std::string& mssFileName : mssFileNames) {
            std::shared_ptr<std::vector<unsigned char>> mssData = _assetLoader->load(mssFileName);
            if (!mssData) {
                throw LoaderException(std::string("Could not load CartoCSS file ") + mssFileName);
            }
            std::string cartoCSS(reinterpret_cast<const char*>(mssData->data()), reinterpret_cast<const char*>(mssData->data() + mssData->size()));
            StyleSheet mssStyleSheet;
            try {
                mssStyleSheet = CartoCSSParser::parse(cartoCSS);
            }
            catch (const CartoCSSParser::ParserError& ex) {
                std::string msg = std::string("Error while parsing file ") + mssFileName;
                if (ex.position().first != 0) {
                    msg += ", error at line " + boost::lexical_cast<std::string>(ex.position().second) + ", column " + boost::lexical_cast<std::string>(ex.position().first);
                }
                msg += std::string(": ") + ex.what();
                throw LoaderException(msg);
            }
            catch (const std::exception& ex) {
                throw LoaderException(std::string("Exception while parsing file ") + mssFileName + ": " + ex.what());
            }
            std::vector<StyleSheet::Element> elements(styleSheet.getElements());
            elements.insert(elements.end(), mssStyleSheet.getElements().begin(), mssStyleSheet.getElements().end());
            styleSheet = StyleSheet(elements);
        }

        // Nutiparameters
        std::vector<mvt::NutiParameter> nutiParameters;
        if (mapDoc.contains("nutiparameters")) {
            const picojson::value::object& nutiparamsObj = mapDoc.get("nutiparameters").get<picojson::value::object>();
            for (auto pit = nutiparamsObj.begin(); pit != nutiparamsObj.end(); pit++) {
                std::string paramName = pit->first;
                const picojson::value& paramValue = pit->second;
                mvt::Value defaultValue = convertJSONValue(paramValue.get("default"));
                std::map<std::string, mvt::Value> enumMap;
                if (paramValue.contains("values")) {
                    const picojson::value::object& valuesObj = paramValue.get("values").get<picojson::value::object>();
                    for (auto vit = valuesObj.begin(); vit != valuesObj.end(); vit++) {
                        enumMap[vit->first] = convertJSONValue(vit->second);
                    }
                }
                nutiParameters.emplace_back(paramName, defaultValue, enumMap);
            }
        }

        return buildMap(styleSheet, layerNames, nutiParameters);
    }

    std::shared_ptr<mvt::Map> CartoCSSMapLoader::buildMap(const StyleSheet& styleSheet, const std::vector<std::string>& layerNames, const std::vector<mvt::NutiParameter>& nutiParameters) const {
        // Map properties
        mvt::Map::Settings mapSettings;
        {
            try {
                ExpressionContext context;

                CartoCSSCompiler compiler(context, false);
                std::map<std::string, Value> mapProperties;
                compiler.compileMap(styleSheet, mapProperties);

                loadMapSettings(mapProperties, mapSettings);
            }
            catch (const std::exception& ex) {
                throw LoaderException(std::string("Error while building/loading map properties: ") + ex.what());
            }
        }
        auto map = std::make_shared<mvt::Map>(mapSettings);
        
        // Set parameters
        map->setNutiParameters(nutiParameters);

        // Layers
        CartoCSSMapnikTranslator translator(_logger);
        for (const std::string& layerName : layerNames) {
            std::map<std::string, AttachmentStyle> attachmentStyleMap;
            for (int zoom = 0; zoom < MAX_ZOOM; zoom++) {
                try {
                    ExpressionContext context;
                    std::map<std::string, Value> predefinedFieldMap;
                    context.predefinedFieldMap = &predefinedFieldMap;
                    (*context.predefinedFieldMap)["zoom"] = Value(static_cast<long long>(zoom));

                    CartoCSSCompiler compiler(context, _ignoreLayerPredicates);
                    std::list<CartoCSSCompiler::LayerAttachment> layerAttachments;
                    compiler.compileLayer(layerName, styleSheet, layerAttachments);

                    buildAttachmentStyleMap(translator, map, zoom, layerAttachments, attachmentStyleMap);
                }
                catch (const std::exception& ex) {
                    throw LoaderException(std::string("Error while building zoom ") + boost::lexical_cast<std::string>(zoom) + " properties: " + ex.what());
                }
            }
            if (attachmentStyleMap.empty()) {
                continue;
            }
            std::vector<AttachmentStyle> attachmentStyles = getSortedAttachmentStyles(attachmentStyleMap);

            std::vector<std::string> styleNames;
            for (const AttachmentStyle& attachmentStyle : attachmentStyles) {
                std::string styleName = layerName + attachmentStyle.attachment;
                auto style = std::make_shared<mvt::Style>(styleName, attachmentStyle.opacity, attachmentStyle.compOp, mvt::Style::FilterMode::FIRST, attachmentStyle.rules);
                map->addStyle(style);
                styleNames.push_back(styleName);
            }
            auto layer = std::make_shared<mvt::Layer>(layerName, styleNames);
            map->addLayer(layer);
        }
        return map;
    }

    void CartoCSSMapLoader::loadMapSettings(const std::map<std::string, Value>& mapProperties, mvt::Map::Settings& mapSettings) const {
        Color backgroundColor;
        if (getMapProperty(mapProperties, "background-color", backgroundColor)) {
            mapSettings.backgroundColor = vt::Color(backgroundColor.value());
        }
        getMapProperty(mapProperties, "background-image", mapSettings.backgroundImage);
        getMapProperty(mapProperties, "font-directory", mapSettings.fontDirectory);
    }

    void CartoCSSMapLoader::buildAttachmentStyleMap(const CartoCSSMapnikTranslator& translator, const std::shared_ptr<mvt::Map>& map, int zoom, const std::list<CartoCSSCompiler::LayerAttachment>& layerAttachments, std::map<std::string, AttachmentStyle>& attachmentStyleMap) const {
        for (const CartoCSSCompiler::LayerAttachment& layerAttachment : layerAttachments) {
            if (attachmentStyleMap.find(layerAttachment.attachment) == attachmentStyleMap.end()) {
                attachmentStyleMap[layerAttachment.attachment].attachment = layerAttachment.attachment;
                attachmentStyleMap[layerAttachment.attachment].order = layerAttachment.order;
            }
            
            AttachmentStyle& attachmentStyle = attachmentStyleMap[layerAttachment.attachment];
            attachmentStyle.order = std::min(attachmentStyle.order, layerAttachment.order);
            for (const CartoCSSCompiler::PropertySet& propertySet : layerAttachment.propertySets) {
                std::shared_ptr<mvt::Rule> rule = translator.buildRule(propertySet, map, zoom);
                if (rule) {
                    attachmentStyle.rules.push_back(rule);
                }

                // Copy opacity and comp-op properties. These are style-level properties, not symbolizer peroperties.
                // Note that we ignore filters, this is CartoCSS design issue and represents how CartoCSS is translated to Mapnik.
                auto opacityIt = propertySet.properties.find("opacity");
                if (opacityIt != propertySet.properties.end()) {
                    attachmentStyle.opacity = boost::lexical_cast<float>(translator.buildExpressionString(opacityIt->second.expression, false));
                }
                auto compOpIt = propertySet.properties.find("comp-op");
                if (compOpIt != propertySet.properties.end()) {
                    attachmentStyle.compOp = translator.buildExpressionString(compOpIt->second.expression, true);
                }
            }
        }
    }

    std::vector<CartoCSSMapLoader::AttachmentStyle> CartoCSSMapLoader::getSortedAttachmentStyles(const std::map<std::string, AttachmentStyle>& attachmentStyleMap) const {
        std::vector<AttachmentStyle> attachmentStyles;
        for (auto it = attachmentStyleMap.begin(); it != attachmentStyleMap.end(); it++) {
            auto insertIt = std::upper_bound(attachmentStyles.begin(), attachmentStyles.end(), it->second, [](const AttachmentStyle& attachmentStyle1, const AttachmentStyle& attachmentStyle2) {
                return attachmentStyle1.order < attachmentStyle2.order;
            });
            attachmentStyles.insert(insertIt, it->second);
        }
        return attachmentStyles;
    }
} }
