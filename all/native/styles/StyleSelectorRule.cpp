#ifdef _CARTO_GDAL_SUPPORT

#include "StyleSelectorRule.h"
#include "search/query/QueryExpression.h"

namespace carto {

    StyleSelectorRule::StyleSelectorRule(const std::shared_ptr<QueryExpression>& expr, const std::shared_ptr<Style>& style) :
        _expression(expr), _style(style)
    {
    }

    StyleSelectorRule::~StyleSelectorRule() {
    }

    const std::shared_ptr<QueryExpression>& StyleSelectorRule::getExpression() const {
        return _expression;
    }
    
    const std::shared_ptr<Style>& StyleSelectorRule::getStyle() const {
        return _style;
    }

}

#endif
