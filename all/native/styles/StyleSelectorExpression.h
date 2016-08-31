/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_STYLESELECTOREXPRESSION_H_
#define _CARTO_STYLESELECTOREXPRESSION_H_

#ifdef _CARTO_GDAL_SUPPORT

#include <memory>

namespace carto {
    class StyleSelectorContext;

    /**
     * Style selector filter expression.
     */
    class StyleSelectorExpression {
    public:
        virtual ~StyleSelectorExpression() { }

        /**
         * Evaluates filter expression given context.
         * @param context The context to use for evaluation
         * @return True or false, depending on the context.
         */
        virtual bool evaluate(const StyleSelectorContext& context) const = 0;
    };
}

#endif

#endif
