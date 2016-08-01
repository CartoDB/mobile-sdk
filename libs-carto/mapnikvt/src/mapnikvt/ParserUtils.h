/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_PARSERUTILS_H_
#define _CARTO_MAPNIKVT_PARSERUTILS_H_

#include "Value.h"
#include "Expression.h"
#include "Transform.h"
#include "vt/Color.h"
#include "vt/TileLayerStyles.h"

#include <stdexcept>
#include <memory>
#include <string>
#include <vector>

namespace carto { namespace mvt {
    class ParserException : public std::runtime_error {
    public:
        explicit ParserException(const std::string& msg) : runtime_error(msg) { }
        explicit ParserException(const std::string& msg, const std::string& source) : runtime_error(msg), _source(source) { }

        const std::string& string() const { return _source; }

    private:
        std::string _source;
    };

    vt::Color parseColor(const std::string& str);
    vt::CompOp parseCompOp(const std::string& str);
    vt::LabelOrientation parseLabelOrientation(const std::string& str);
    Value parseValue(const std::string& str);
    std::shared_ptr<Expression> parseExpression(const std::string& str);
    std::shared_ptr<Expression> parseStringExpression(const std::string& str);
    std::vector<std::shared_ptr<Transform>> parseTransformList(const std::string& str);
} }

#endif
