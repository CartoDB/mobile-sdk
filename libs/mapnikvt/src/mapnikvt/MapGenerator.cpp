#include "MapGenerator.h"
#include "Map.h"
#include "FontSet.h"
#include "Expression.h"
#include "Predicate.h"
#include "Filter.h"
#include "Rule.h"
#include "Style.h"
#include "Layer.h"
#include "Map.h"
#include "GeneratorUtils.h"
#include "Symbolizer.h"
#include "SymbolizerGenerator.h"
#include "ScaleUtils.h"
#include "Logger.h"

namespace carto { namespace mvt {
    struct MapGenerator::TypeExtractor : boost::static_visitor<std::string> {
        std::string operator() (bool) const { return "bool"; }
        std::string operator() (double) const { return "float"; }
        std::string operator() (long long) const { return "int"; }
        std::string operator() (const std::string&) const { return "string"; }
        template <typename T> std::string operator() (T) const { return ""; }
    };

    std::shared_ptr<pugi::xml_document> MapGenerator::generateMap(const Map& map) const {
        auto doc = std::make_shared<pugi::xml_document>();
        pugi::xml_node mapNode = doc->append_child("Map");

        Map::Settings mapSettings = map.getSettings();
        mapNode.append_attribute("font-directory").set_value(mapSettings.fontDirectory.c_str());
        mapNode.append_attribute("background-color").set_value(generateColorString(mapSettings.backgroundColor).c_str());
        mapNode.append_attribute("background-image").set_value(mapSettings.backgroundImage.c_str());

        // Parameters
        pugi::xml_node paramsNode = mapNode.append_child("Parameters");
        for (auto it = map.getParameterMap().begin(); it != map.getParameterMap().end(); it++) {
            const Parameter& param = it->second;
            pugi::xml_node paramNode = paramsNode.append_child("Parameter");
            paramNode.append_attribute("name").set_value(param.getName().c_str());
            paramNode.append_child(pugi::node_pcdata).set_value(param.getValue().c_str());
        }

        // NutiParameters
        pugi::xml_node nutiParamsNode = mapNode.append_child("NutiParameters");
        for (auto it = map.getNutiParameterMap().begin(); it != map.getNutiParameterMap().end(); it++) {
            const NutiParameter& nutiParam = it->second;
            pugi::xml_node nutiParamNode = nutiParamsNode.append_child("NutiParameter");
            nutiParamNode.append_attribute("name").set_value(nutiParam.getName().c_str());
            nutiParamNode.append_attribute("type").set_value(generateTypeString(nutiParam.getDefaultValue()).c_str());
            nutiParamNode.append_attribute("value").set_value(ValueConverter<std::string>::convert(nutiParam.getDefaultValue()).c_str());

            for (auto it2 = nutiParam.getEnumMap().begin(); it2 != nutiParam.getEnumMap().end(); it2++) {
                pugi::xml_node valueNode = nutiParamNode.append_child("Value");
                valueNode.append_attribute("id").set_value(it2->first.c_str());
                valueNode.append_child(pugi::node_pcdata).set_value(ValueConverter<std::string>::convert(it2->second).c_str());
            }
        }

        // FontSets
        for (auto it = map.getFontSets().begin(); it != map.getFontSets().end(); it++) {
            const FontSet& fontSet = **it;
            pugi::xml_node fontSetNode = mapNode.append_child("FontSet");
            fontSetNode.append_attribute("name").set_value(fontSet.getName().c_str());
            for (const std::string& faceName : fontSet.getFaceNames()) {
                pugi::xml_node fontNode = fontSetNode.append_child("Font");
                fontNode.append_attribute("face-name").set_value(faceName.c_str());
            }
        }

        // Styles
        for (auto it = map.getStyles().begin(); it != map.getStyles().end(); it++) {
            const Style& style = **it;
            pugi::xml_node styleNode = mapNode.append_child("Style");
            styleNode.append_attribute("name").set_value(style.getName().c_str());
            if (style.getOpacity() != 1.0f) {
                styleNode.append_attribute("opacity").set_value(style.getOpacity());
            }
            if (!style.getCompOp().empty()) {
                styleNode.append_attribute("comp-op").set_value(style.getCompOp().c_str());
            }

            switch (style.getFilterMode())
            {
            case Style::FilterMode::FIRST:
                styleNode.append_attribute("filter-mode").set_value("first");
                break;
            default:
                _logger->write(Logger::Severity::WARNING, "Unsupported filter mode");
                break;
            }

            for (auto it2 = style.getRules().begin(); it2 != style.getRules().end(); it2++) {
                const Rule& rule = **it2;
                pugi::xml_node ruleNode = styleNode.append_child("Rule");
                ruleNode.append_attribute("name").set_value(rule.getName().c_str());
                
                ruleNode.append_child("MinScaleDenominator").append_child(pugi::node_pcdata).set_value(boost::lexical_cast<std::string>(zoom2ScaleDenominator(rule.getMaxZoom())).c_str());
                ruleNode.append_child("MaxScaleDenominator").append_child(pugi::node_pcdata).set_value(boost::lexical_cast<std::string>(zoom2ScaleDenominator(rule.getMinZoom())).c_str());
                
                if (std::shared_ptr<Filter> filter = rule.getFilter()) {
                    pugi::xml_node filterNode;
                    switch (filter->getType()) {
                    case Filter::Type::FILTER:
                        filterNode = ruleNode.append_child("Filter");
                        break;
                    case Filter::Type::ELSEFILTER:
                        filterNode = ruleNode.append_child("ElseFilter");
                        break;
                    case Filter::Type::ALSOFILTER:
                        filterNode = ruleNode.append_child("AlsoFilter");
                        break;
                    }
                    if (std::shared_ptr<Predicate> pred = filter->getPredicate()) {
                        filterNode.append_child(pugi::node_pcdata).set_value(generateExpressionString(std::make_shared<PredicateExpression>(pred)).c_str());
                    }
                }

                for (auto it3 = rule.getSymbolizers().begin(); it3 != rule.getSymbolizers().end(); it3++) {
                    const Symbolizer& symbolizer = **it3;
                    std::shared_ptr<pugi::xml_node> symbolizerNode = _symbolizerGenerator->generateSymbolizer(symbolizer);
                    if (symbolizerNode) {
                        ruleNode.append_copy(*symbolizerNode);
                    }
                }
            }
        }

        // Layers
        for (auto it = map.getLayers().begin(); it != map.getLayers().end(); it++) {
            const Layer& layer = **it;
            pugi::xml_node layerNode = mapNode.append_child("Layer");
            layerNode.append_attribute("name").set_value(layer.getName().c_str());

            for (auto it2 = layer.getStyleNames().begin(); it2 != layer.getStyleNames().end(); it2++) {
                layerNode.append_child("StyleName").append_child(pugi::node_pcdata).set_value((*it2).c_str());
            }
        }

        return doc;
    }

    std::string MapGenerator::generateTypeString(const Value& value) const {
        std::string typeString = boost::apply_visitor(TypeExtractor(), value);
        if (typeString.empty()) {
            _logger->write(Logger::Severity::WARNING, "Unsupported value type");
        }
        return typeString;
    }
} }
