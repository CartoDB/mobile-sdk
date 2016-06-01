#include "StyleSelectorRule.h"

namespace carto {

    StyleSelectorRule::StyleSelectorRule(const std::shared_ptr<StyleSelectorExpression>& expr, const std::shared_ptr<Style>& style) :
        _expression(expr), _style(style)
    {
    }

    StyleSelectorRule::~StyleSelectorRule() {
    }

    const std::shared_ptr<StyleSelectorExpression>& StyleSelectorRule::getExpression() const {
        return _expression;
    }
    
    const std::shared_ptr<Style>& StyleSelectorRule::getStyle() const {
        return _style;
    }

}
