#include "Map.h"
#include "SymbolizerParser.h"
#include "PointSymbolizer.h"
#include "LineSymbolizer.h"
#include "LinePatternSymbolizer.h"
#include "PolygonSymbolizer.h"
#include "PolygonPatternSymbolizer.h"
#include "BuildingSymbolizer.h"
#include "MarkersSymbolizer.h"
#include "TextSymbolizer.h"
#include "ShieldSymbolizer.h"
#include "ParserUtils.h"
#include "Logger.h"

#include <boost/algorithm/string.hpp>

namespace carto { namespace mvt {
    std::shared_ptr<Symbolizer> SymbolizerParser::parseSymbolizer(const pugi::xml_node& node, const std::shared_ptr<Map>& map) const {
        std::shared_ptr<Symbolizer> symbolizer = createSymbolizer(node, map);
        if (!symbolizer) {
            return symbolizer;
        }
        
        pugi::xpath_node_set parameterNodes = pugi::xpath_query("CssParameter").evaluate_node_set(node);
        for (pugi::xpath_node_set::const_iterator parameterIt = parameterNodes.begin(); parameterIt != parameterNodes.end(); ++parameterIt) {
            pugi::xml_node parameterNode = (*parameterIt).node();
            std::string parameterName = boost::replace_all_copy(std::string(parameterNode.attribute("name").as_string()), "_", "-");
            std::string parameterValue = parameterNode.text().as_string();
            try {
                symbolizer->setParameter(parameterName, parameterValue);
            }
            catch (const std::runtime_error& ex) {
                _logger->write(mvt::Logger::Severity::ERROR, ex.what());
            }
        }
        for (pugi::xml_attribute_iterator attrIt = node.attributes().begin(); attrIt != node.attributes().end(); ++attrIt) {
            pugi::xml_attribute attr = *attrIt;
            std::string parameterName = boost::replace_all_copy(std::string(attr.name()), "_", "-");
            std::string parameterValue = attr.as_string();
            try {
                symbolizer->setParameter(parameterName, parameterValue);
            }
            catch (const std::runtime_error& ex) {
                _logger->write(mvt::Logger::Severity::ERROR, ex.what());
            }
        }

        return symbolizer;
    }
    
    std::shared_ptr<Symbolizer> SymbolizerParser::createSymbolizer(const pugi::xml_node& node, const std::shared_ptr<Map>& map) const {
        std::string type = node.name();

        std::shared_ptr<Symbolizer> symbolizer;
        if (type == "PointSymbolizer") {
            symbolizer = std::make_shared<PointSymbolizer>(_logger);
        }
        else if (type == "LineSymbolizer") {
            symbolizer = std::make_shared<LineSymbolizer>(_logger);
        }
        else if (type == "LinePatternSymbolizer") {
            symbolizer = std::make_shared<LinePatternSymbolizer>(_logger);
        }
        else if (type == "PolygonSymbolizer") {
            symbolizer = std::make_shared<PolygonSymbolizer>(_logger);
        }
        else if (type == "PolygonPatternSymbolizer") {
            symbolizer = std::make_shared<PolygonPatternSymbolizer>(_logger);
        }
        else if (type == "BuildingSymbolizer") {
            symbolizer = std::make_shared<BuildingSymbolizer>(_logger);
        }
        else if (type == "MarkersSymbolizer") {
            symbolizer = std::make_shared<MarkersSymbolizer>(_logger);
        }
        else if (type == "ShieldSymbolizer" || type == "TextSymbolizer") {
            std::shared_ptr<Expression> textExpr;
            if (!node.text().empty()) {
                std::string text = node.text().as_string();
                if (!text.empty()) {
                    try {
                        textExpr = parseExpression(text);
                    }
                    catch (const std::runtime_error& ex) {
                        _logger->write(mvt::Logger::Severity::ERROR, ex.what());
                        return std::shared_ptr<Symbolizer>();
                    }
                }
            }
            if (!textExpr) {
                _logger->write(Logger::Severity::WARNING, "Missing text expression: " + type);
                return std::shared_ptr<Symbolizer>();
            }

            if (type == "ShieldSymbolizer") {
                symbolizer = std::make_shared<ShieldSymbolizer>(map->getFontSets(), _logger);
            }
            else {
                symbolizer = std::make_shared<TextSymbolizer>(map->getFontSets(), _logger);
            }
            std::static_pointer_cast<TextSymbolizer>(symbolizer)->setTextExpression(textExpr);
        }
        else {
            _logger->write(Logger::Severity::WARNING, "Unsupported symbolizer type: " + type);
        }
        return symbolizer;
    }
} }
