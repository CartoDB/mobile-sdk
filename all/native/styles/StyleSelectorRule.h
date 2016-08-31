/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_STYLESELECTORRULE_H_
#define _CARTO_STYLESELECTORRULE_H_

#ifdef _CARTO_GDAL_SUPPORT

#include <memory>

namespace carto {
    class Style;
    class StyleSelectorExpression;

    /**
     * Style selector rule. Rule is a combination of the optional filter and style.
     * If filter is present, then filter is evaluated to check if the style can be applied.
     */
    class StyleSelectorRule {
    public:
        /**
         * Constructs a new style selector rule, given filter expression and style.
         * @param expr The filter expression. Can be null pointer.
         * @param style The style of the rule.
         */
        StyleSelectorRule(const std::shared_ptr<StyleSelectorExpression>& expr, const std::shared_ptr<Style>& style);
        virtual ~StyleSelectorRule();

        /**
         * Returns filter expression of this rule. Result can be null pointer.
         * @return The filter expression or null, if none is specified.
         */
        const std::shared_ptr<StyleSelectorExpression>& getExpression() const;
        /**
         * Returns the style associated with this rule.
         * @return The style.
         */
        const std::shared_ptr<Style>& getStyle() const;

    private:
        std::shared_ptr<StyleSelectorExpression> _expression;
        std::shared_ptr<Style> _style;
    };
}

#endif

#endif
