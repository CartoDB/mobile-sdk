/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_GENERATORUTILS_H_
#define _CARTO_MAPNIKVT_GENERATORUTILS_H_

#include "Value.h"
#include "Expression.h"
#include "Transform.h"
#include "vt/Color.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace carto { namespace mvt {
    class GeneratorException : public std::runtime_error {
    public:
        explicit GeneratorException(const std::string& msg) : runtime_error(msg) { }
    };

    std::string generateColorString(vt::Color color);
    std::string generateValueString(const Value& val);
    std::string generateExpressionString(const std::shared_ptr<const Expression>& expr);
    std::string generateStringExpressionString(const std::shared_ptr<const Expression>& expr);
    std::string generateTransformListString(const std::vector<std::shared_ptr<const Transform>>& transforms);
} }

#endif
