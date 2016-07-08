#ifdef _CARTO_GDAL_SUPPORT

#include "StyleSelectorBuilder.h"
#include "StyleSelector.h"
#include "StyleSelectorRule.h"
#include "StyleSelectorExpression.h"
#include "StyleSelectorExpressionParser.h"
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
        std::string::const_iterator it = expr.begin();
        std::string::const_iterator end = expr.end();
        StyleSelectorExpressionImpl::encoding::space_type space;
        std::shared_ptr<StyleSelectorExpression> ruleExpr;
        bool result = boost::spirit::qi::phrase_parse(it, end, StyleSelectorExpressionParser<std::string::const_iterator>(), space, ruleExpr);
        if (!result) {
            Log::Error("StyleSelectorBuilder: Failed to parse filter expression.");
        } else if (it != expr.end()) {
            Log::Error("StyleSelectorBuilder: Could not parse to the end of filter expression.");
        } else {
            _rules.emplace_back(std::make_shared<StyleSelectorRule>(ruleExpr, style));
        }
    }
    
    void StyleSelectorBuilder::addRule(const std::shared_ptr<Style>& style) {
        _rules.emplace_back(std::make_shared<StyleSelectorRule>(std::shared_ptr<StyleSelectorExpression>(), style));
    }
    
    std::shared_ptr<StyleSelector> StyleSelectorBuilder::buildSelector() const {
        return std::make_shared<StyleSelector>(_rules);
    }

}

#endif
