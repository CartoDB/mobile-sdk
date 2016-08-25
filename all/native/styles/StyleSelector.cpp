#ifdef _CARTO_GDAL_SUPPORT

#include "StyleSelector.h"
#include "styles/StyleSelectorRule.h"
#include "styles/StyleSelectorExpression.h"
#include "styles/StyleSelectorContext.h"

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

#endif
