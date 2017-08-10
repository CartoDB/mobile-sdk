/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_QUERYEXPRESSION_H_
#define _CARTO_QUERYEXPRESSION_H_

#include <memory>

namespace carto {
    class QueryContext;

    /**
     * Query filter expression.
     */
    class QueryExpression {
    public:
        virtual ~QueryExpression() { }

        /**
         * Evaluates filter expression given context.
         * @param context The context to use for evaluation
         * @return True or false, depending on the context.
         */
        virtual bool evaluate(const QueryContext& context) const = 0;
    };
}

#endif
