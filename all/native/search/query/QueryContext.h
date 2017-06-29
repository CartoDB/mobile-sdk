/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_QUERYCONTEXT_H_
#define _CARTO_QUERYCONTEXT_H_

#include "core/Variant.h"

#include <memory>
#include <string>
#include <map>

namespace carto {

    /**
     * Query context for evaluating rules.
     */
    class QueryContext {
    public:
        virtual ~QueryContext() { }

        /**
         * Tries to find variable value based on its name.
         * @param name The name of the context variable
         * @param value The corresponding value, used as an output parameter
         * @return True if variable name was matched and its value was assigned to value parameter, false otherwise.
         */
        virtual bool getVariable(const std::string& name, Variant& value) const = 0;
    };
}

#endif
