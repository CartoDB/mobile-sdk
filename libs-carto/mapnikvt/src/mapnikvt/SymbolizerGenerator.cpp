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
    void SymbolizerGenerator::generateSymbolizer(const Symbolizer& symbolizer, pugi::xml_node& symbolizerNode) const {
        std::string type;
        std::shared_ptr<Expression> expr;
        if (dynamic_cast<const PointSymbolizer*>(&symbolizer)) {
            type = "PointSymbolizer";
        }
        else if (dynamic_cast<const LineSymbolizer*>(&symbolizer)) {
            type = "LineSymbolizer";
        }
        else if (dynamic_cast<const LinePatternSymbolizer*>(&symbolizer)) {
            type = "LinePatternSymbolizer";
        }
        else if (dynamic_cast<const PolygonSymbolizer*>(&symbolizer)) {
            type = "PolygonSymbolizer";
        }
        else if (dynamic_cast<const PolygonPatternSymbolizer*>(&symbolizer)) {
            type = "PolygonPatternSymbolizer";
        }
        else if (dynamic_cast<const BuildingSymbolizer*>(&symbolizer)) {
            type = "BuildingSymbolizer";
        }
        else if (dynamic_cast<const MarkersSymbolizer*>(&symbolizer)) {
            type = "MarkersSymbolizer";
        }
        else if (auto textSymbolizer = dynamic_cast<const TextSymbolizer*>(&symbolizer)) {
            if (dynamic_cast<const ShieldSymbolizer*>(&symbolizer)) {
                type = "ShieldSymbolizer";
            }
            else {
                type = "TextSymbolizer";
            }
            expr = textSymbolizer->getTextExpression();
        }

        symbolizerNode.set_name(type.c_str());
        for (auto it = symbolizer.getParameterMap().begin(); it != symbolizer.getParameterMap().end(); it++) {
            symbolizerNode.append_attribute(it->first.c_str()).set_value(it->second.c_str());
        }
        if (expr) {
            symbolizerNode.set_value(generateExpressionString(expr).c_str());
        }
    }
} }
