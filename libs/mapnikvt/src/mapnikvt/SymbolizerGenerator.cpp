#include "SymbolizerGenerator.h"
#include "PointSymbolizer.h"
#include "LineSymbolizer.h"
#include "LinePatternSymbolizer.h"
#include "PolygonSymbolizer.h"
#include "PolygonPatternSymbolizer.h"
#include "BuildingSymbolizer.h"
#include "MarkersSymbolizer.h"
#include "TextSymbolizer.h"
#include "ShieldSymbolizer.h"
#include "GeneratorUtils.h"
#include "Logger.h"

namespace carto { namespace mvt {
    std::shared_ptr<pugi::xml_node> SymbolizerGenerator::generateSymbolizer(const Symbolizer& symbolizer) const {
        std::string type;
        std::shared_ptr<Expression> expr;
        if (auto pointSymbolizer = dynamic_cast<const PointSymbolizer*>(&symbolizer)) {
            type = "PointSymbolizer";
        }
        else if (auto lineSymbolizer = dynamic_cast<const LineSymbolizer*>(&symbolizer)) {
            type = "LineSymbolizer";
        }
        else if (auto linePatternSymbolizer = dynamic_cast<const LinePatternSymbolizer*>(&symbolizer)) {
            type = "LinePatternSymbolizer";
        }
        else if (auto polygonSymbolizer = dynamic_cast<const PolygonSymbolizer*>(&symbolizer)) {
            type = "PolygonSymbolizer";
        }
        else if (auto polygonPatternSymbolizer = dynamic_cast<const PolygonPatternSymbolizer*>(&symbolizer)) {
            type = "PolygonPatternSymbolizer";
        }
        else if (auto buildingSymbolizer = dynamic_cast<const BuildingSymbolizer*>(&symbolizer)) {
            type = "BuildingSymbolizer";
        }
        else if (auto markersSymbolizer = dynamic_cast<const MarkersSymbolizer*>(&symbolizer)) {
            type = "MarkersSymbolizer";
        }
        else if (auto textSymbolizer = dynamic_cast<const TextSymbolizer*>(&symbolizer)) {
            if (auto shieldSymbolizer = dynamic_cast<const ShieldSymbolizer*>(&symbolizer)) {
                type = "ShieldSymbolizer";
            }
            else {
                type = "TextSymbolizer";
            }
            expr = textSymbolizer->getTextExpression();
        }

        auto node = std::make_shared<pugi::xml_node>();
        node->set_name(type.c_str());
        for (auto it = symbolizer.getParameterMap().begin(); it != symbolizer.getParameterMap().end(); it++) {
            node->append_attribute(it->first.c_str()).set_value(it->second.c_str());
        }
        if (expr) {
            node->set_value(generateExpressionString(expr).c_str());
        }
        return node;
    }
} }
