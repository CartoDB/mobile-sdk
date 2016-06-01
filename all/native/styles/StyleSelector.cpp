#include "StyleSelector.h"
#include "StyleSelectorRule.h"
#include "StyleSelectorExpression.h"
#include "StyleSelectorContext.h"

namespace carto {

    StyleSelector::StyleSelector(const std::vector<std::shared_ptr<StyleSelectorRule> >& rules) :
        _rules(rules)
    {
    }

    StyleSelector::~StyleSelector() {
    }

    const std::shared_ptr<Style>& StyleSelector::getStyle(const StyleSelectorContext& context) const {
        static std::shared_ptr<Style> nullStyle;
        for (const std::shared_ptr<StyleSelectorRule>& rule : _rules) {
            if (std::shared_ptr<StyleSelectorExpression> expr = rule->getExpression()) {
                if (expr->evaluate(context)) {
                    return rule->getStyle();
                }
            }
            else {
                return rule->getStyle();
            }
        }
        return nullStyle;
    }

}
