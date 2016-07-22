#include "MapParser.h"
#include "Map.h"
#include "FontSet.h"
#include "Expression.h"
#include "Predicate.h"
#include "Filter.h"
#include "Rule.h"
#include "Style.h"
#include "Layer.h"
#include "Map.h"
#include "ParserUtils.h"
#include "Symbolizer.h"
#include "SymbolizerParser.h"
#include "ScaleUtils.h"
#include "Logger.h"

namespace carto { namespace mvt {
    std::shared_ptr<Map> MapParser::parseMap(const pugi::xml_document& doc) const {
        pugi::xpath_node_set mapNodes = pugi::xpath_query("Map").evaluate_node_set(doc);
        if (mapNodes.size() != 1) {
            throw ParserException("Stylesheet must contain only single Map element");
        }
        pugi::xml_node mapNode = (*mapNodes.begin()).node();

        // Basic attributes
        Map::Settings mapSettings;
        if (pugi::xml_attribute fontDirectory = mapNode.attribute("font-directory")) {
            mapSettings.fontDirectory = fontDirectory.as_string();
        }
        if (pugi::xml_attribute bgColor = mapNode.attribute("bgcolor")) {
            mapSettings.backgroundColor = parseColor(bgColor.as_string());
        }
        if (pugi::xml_attribute backgroundColor = mapNode.attribute("background-color")) {
            mapSettings.backgroundColor = parseColor(backgroundColor.as_string());
        }
        if (pugi::xml_attribute backgroundImage = mapNode.attribute("background-image")) {
            mapSettings.backgroundImage = backgroundImage.as_string();
        }

        // Build map
        auto map = std::make_shared<Map>(mapSettings);

        // Parameters
        std::vector<Parameter> parameters;
        pugi::xpath_node_set paramNodes = pugi::xpath_query("Parameters/Parameter").evaluate_node_set(mapNode);
        for (pugi::xpath_node_set::const_iterator paramIt = paramNodes.begin(); paramIt != paramNodes.end(); ++paramIt) {
            pugi::xml_node parameterNode = (*paramIt).node();
            std::string name = parameterNode.attribute("name").as_string();
            std::string value = parameterNode.text().as_string();
            parameters.emplace_back(name, value);
        }
        map->setParameters(parameters);

        // NutiParameters
        std::vector<NutiParameter> nutiParameters;
        pugi::xpath_node_set nutiParamNodes = pugi::xpath_query("NutiParameters/NutiParameter").evaluate_node_set(mapNode);
        for (pugi::xpath_node_set::const_iterator nutiParamIt = nutiParamNodes.begin(); nutiParamIt != nutiParamNodes.end(); ++nutiParamIt) {
            pugi::xml_node parameterNode = (*nutiParamIt).node();
            std::string name = parameterNode.attribute("name").as_string();
            std::string type = parameterNode.attribute("type").as_string();
            Value defaultValue = parseTypedValue(type, parameterNode.attribute("value").as_string());
            std::map<std::string, Value> enumMap;
            pugi::xpath_node_set valueNodes = pugi::xpath_query("Value").evaluate_node_set(parameterNode);
            for (pugi::xpath_node_set::const_iterator valueIt = valueNodes.begin(); valueIt != valueNodes.end(); ++valueIt) {
                pugi::xml_node valueNode = (*valueIt).node();
                std::string id = valueNode.attribute("id").as_string();
                enumMap[id] = parseTypedValue(type, valueNode.attribute("value").as_string());
            }
            nutiParameters.emplace_back(name, defaultValue, enumMap);
        }
        map->setNutiParameters(nutiParameters);

        // FontSets
        pugi::xpath_node_set fontSetNodes = pugi::xpath_query("FontSet").evaluate_node_set(mapNode);
        for (pugi::xpath_node_set::const_iterator fontSetIt = fontSetNodes.begin(); fontSetIt != fontSetNodes.end(); ++fontSetIt) {
            pugi::xml_node fontSetNode = (*fontSetIt).node();
            pugi::xml_attribute fontSetName = fontSetNode.attribute("name");

            std::vector<std::string> faceNames;
            pugi::xpath_node_set fontNodes = pugi::xpath_query("Font").evaluate_node_set(fontSetNode);
            for (pugi::xpath_node_set::const_iterator fontIt = fontNodes.begin(); fontIt != fontNodes.end(); ++fontIt) {
                pugi::xml_node fontNode = (*fontIt).node();
                pugi::xml_attribute faceName = fontNode.attribute("face-name");
                faceNames.push_back(faceName.as_string());
            }
            auto fontSet = std::make_shared<FontSet>(fontSetName.as_string(), faceNames);
            map->addFontSet(fontSet);
        }

        // Styles
        pugi::xpath_node_set styleNodes = pugi::xpath_query("Style").evaluate_node_set(mapNode);
        for (pugi::xpath_node_set::const_iterator styleIt = styleNodes.begin(); styleIt != styleNodes.end(); ++styleIt) {
            pugi::xml_node styleNode = (*styleIt).node();
            std::string styleName = styleNode.attribute("name").as_string();
            float styleOpacity = styleNode.attribute("opacity").as_float(1.0f);
            std::string compOp = styleNode.attribute("comp-op").as_string("");

            Style::FilterMode filterMode = Style::FilterMode::ALL;
            if (styleNode.attribute("filter-mode")) {
                std::string filterModeString = styleNode.attribute("filter-mode").as_string();
                if (filterModeString == "first") {
                    filterMode = Style::FilterMode::FIRST;
                }
                else if (filterModeString != "") {
                    _logger->write(Logger::Severity::WARNING, "Unsupported filter mode: " + filterModeString);
                }
            }

            std::vector<std::shared_ptr<const Rule>> rules;
            pugi::xpath_node_set ruleNodes = pugi::xpath_query("Rule").evaluate_node_set(styleNode);
            for (pugi::xpath_node_set::const_iterator ruleIt = ruleNodes.begin(); ruleIt != ruleNodes.end(); ++ruleIt) {
                pugi::xml_node ruleNode = (*ruleIt).node();
                std::string ruleName = ruleNode.attribute("name").as_string();
                float minScaleDenominator = 0;
                float maxScaleDenominator = std::numeric_limits<float>::infinity();
                std::shared_ptr<const Filter> filter;
                std::vector<std::shared_ptr<Symbolizer>> symbolizers;

                for (pugi::xml_node_iterator nodeIt = ruleNode.children().begin(); nodeIt != ruleNode.children().end(); ++nodeIt) {
                    pugi::xml_node node = *nodeIt;
                    std::string nodeName = node.name();
                    if (nodeName == "MinScaleDenominator") {
                        minScaleDenominator = node.text().as_float();
                    }
                    else if (nodeName == "MaxScaleDenominator") {
                        maxScaleDenominator = node.text().as_float();
                    }
                    else if (nodeName == "Filter") {
                        std::string exprStr = node.text().as_string();
                        if (!exprStr.empty()) {
                            std::shared_ptr<Expression> expr = parseExpression(exprStr);
                            std::shared_ptr<const Predicate> pred;
                            if (auto predExpr = std::dynamic_pointer_cast<PredicateExpression>(expr)) {
                                pred = predExpr->getPredicate();
                            } else {
                                pred = std::make_shared<ExpressionPredicate>(expr);
                            }
                            filter = std::make_shared<Filter>(Filter::Type::FILTER, pred);
                        }
                    }
                    else if (nodeName == "ElseFilter") {
                        filter = std::make_shared<Filter>(Filter::Type::ELSEFILTER, std::shared_ptr<Predicate>());
                    }
                    else if (nodeName == "AlsoFilter") {
                        filter = std::make_shared<Filter>(Filter::Type::ALSOFILTER, std::shared_ptr<Predicate>());
                    }
                    else {
                        std::shared_ptr<Symbolizer> symbolizer = _symbolizerParser->parseSymbolizer(node, map);
                        if (!symbolizer) {
                            continue;
                        }
                        symbolizers.push_back(symbolizer);
                    }
                }

                float maxScaleDenominatorZoom = std::max(0.0f, scaleDenominator2Zoom(maxScaleDenominator));
                float minScaleDenominatorZoom = std::min(64.0f, scaleDenominator2Zoom(minScaleDenominator));
                int minZoom = 64;
                int maxZoom = 0;
                for (int zoom = static_cast<int>(std::floor(maxScaleDenominatorZoom)); zoom <= static_cast<int>(std::ceil(minScaleDenominatorZoom)); zoom++) {
                    float scaleDenom = zoom2ScaleDenominator(static_cast<float>(zoom));
                    if (scaleDenom >= minScaleDenominator && scaleDenom < maxScaleDenominator) {
                        minZoom = std::min(minZoom, zoom);
                        maxZoom = std::max(maxZoom, zoom);
                    }
                }
                auto rule = std::make_shared<Rule>(ruleName, minZoom, maxZoom + 1, filter, symbolizers);
                rules.push_back(rule);
            }

            auto style = std::make_shared<Style>(styleName, styleOpacity, compOp, filterMode, rules);
            map->addStyle(style);
        }

        // Layers
        pugi::xpath_node_set layerNodes = pugi::xpath_query("Layer").evaluate_node_set(mapNode);
        for (pugi::xpath_node_set::const_iterator layerIt = layerNodes.begin(); layerIt != layerNodes.end(); ++layerIt) {
            pugi::xml_node layerNode = (*layerIt).node();
            std::string layerName = layerNode.attribute("name").as_string();
            if (layerNode.attribute("group-by")) {
                std::string groupByString = layerNode.attribute("group-by").as_string();
                _logger->write(Logger::Severity::WARNING, "Unsupported layer group-by mode: " + groupByString);
            }

            std::vector<std::string> styleNames;
            pugi::xpath_node_set styleNameNodes = pugi::xpath_query("StyleName").evaluate_node_set(layerNode);
            for (pugi::xpath_node_set::const_iterator styleNameIt = styleNameNodes.begin(); styleNameIt != styleNameNodes.end(); ++styleNameIt) {
                pugi::xml_node styleNameNode = (*styleNameIt).node();
                std::string styleName = styleNameNode.text().as_string();
                styleNames.push_back(styleName);
            }

            auto layer = std::make_shared<Layer>(layerName, styleNames);
            map->addLayer(layer);
        }

        return map;
    }

    Value MapParser::parseTypedValue(const std::string& type, const std::string& value) const {
        try {
            if (type == "bool") {
                if (value == "true") {
                    return Value(true);
                }
                if (value == "false") {
                    return Value(false);
                }
                return Value(boost::lexical_cast<bool>(value));
            }
            else if (type == "int") {
                return Value(boost::lexical_cast<long long>(value));
            }
            else if (type == "float") {
                return Value(boost::lexical_cast<double>(value));
            }
            else if (type == "string") {
                return Value(value);
            }
            else {
                _logger->write(Logger::Severity::WARNING, "Unsupported value type: " + type);
            }
        }
        catch (const boost::bad_numeric_cast&) {
            _logger->write(Logger::Severity::ERROR, "Could not convert value: " + value + " to: " + type);
        }
        return Value();
    }
} }
