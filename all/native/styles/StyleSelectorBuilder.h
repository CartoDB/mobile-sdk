/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_STYLESELECTORBUILDER_H_
#define _CARTO_STYLESELECTORBUILDER_H_

#ifdef _CARTO_GDAL_SUPPORT

#include <memory>
#include <string>
#include <vector>

namespace carto {
    class Style;
    class StyleSelector;
    class StyleSelectorRule;

    /**
     * Builder class for style selectors.
     */
    class StyleSelectorBuilder {
    public:
        /**
         * Constructs a new empty builder.
         */
        StyleSelectorBuilder();
        virtual ~StyleSelectorBuilder();

        /**
         * Adds new rule to the builder rule set.
         * @param rule The rule to add
         * @return Self.
         */
        void addRule(const std::shared_ptr<StyleSelectorRule>& rule);

        /**
         * Adds new rule to the builder rule set. The rules are defined by a filter expression and corresponding style.
         * Filter expressions are defined using simple built-in SQL like language with the following properties:
         * - The following boolean operators are supported: AND, OR, NOT.
         * - The following predicates are supported: IS NULL, IS NOT NULL, =, <>, <=, >=, <, >.
         * - Operator precedence is similar to SQL (NOT > AND > OR), parenthesis can be used to group expressions.
         * - There are few prebuilt variables (view::zoom, geometry::type), other variables correspond to metadata values.
         * @param expr The filter expression to use for this rule. For example, "geometry::type = 'linestring' AND type = 'road'"
         * @param style The style for this rule.
         * @return Self.
         */
        void addRule(const std::string& expr, const std::shared_ptr<Style>& style);

        /**
         * Adds new rule to the builder rule set. This method should be used to add the 'default' style, if no filter needs to be applied.
         * @param style The style for this rule.
         * @return Self.
         */
        void addRule(const std::shared_ptr<Style>& style);
        
        /**
         * Builds a new style selector from the added rule list.
         * @return Style selector with all the added rules.
         */
        std::shared_ptr<StyleSelector> buildSelector() const;

    private:
        std::vector<std::shared_ptr<StyleSelectorRule> > _rules;
    };
}

#endif

#endif
