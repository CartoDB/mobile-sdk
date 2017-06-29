#ifdef _CARTO_GDAL_SUPPORT

#include "StyleSelectorBuilder.h"
#include "StyleSelector.h"
#include "StyleSelectorRule.h"
#include "search/query/QueryExpression.h"
#include "search/query/QueryExpressionParser.h"
#include "utils/Log.h"

namespace carto {

    StyleSelectorBuilder::StyleSelectorBuilder() {
    }

    StyleSelectorBuilder::~StyleSelectorBuilder() {
    }

    void StyleSelectorBuilder::addRule(const std::shared_ptr<StyleSelectorRule>& rule) {
        _rules.emplace_back(rule);
    }
    
    void StyleSelectorBuilder::addRule(const std::string& expr, const std::shared_ptr<Style>& style) {
        std::shared_ptr<QueryExpression> queryExpr = QueryExpressionParser::parse(expr);
        _rules.emplace_back(std::make_shared<StyleSelectorRule>(queryExpr, style));
    }
    
    void StyleSelectorBuilder::addRule(const std::shared_ptr<Style>& style) {
        _rules.emplace_back(std::make_shared<StyleSelectorRule>(std::shared_ptr<QueryExpression>(), style));
    }
    
    std::shared_ptr<StyleSelector> StyleSelectorBuilder::buildSelector() const {
        return std::make_shared<StyleSelector>(_rules);
    }

}

#endif
