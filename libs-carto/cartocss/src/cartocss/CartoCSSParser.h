/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSS_CARTOCSSPARSER_H_
#define _CARTO_CARTOCSS_CARTOCSSPARSER_H_

#include "Expression.h"
#include "Predicate.h"
#include "StyleSheet.h"

#include <memory>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <stdexcept>

namespace carto { namespace css {
    class CartoCSSParser final {
    public:
        struct ParserError : std::runtime_error {
            explicit ParserError(const std::string& msg) : runtime_error(msg), _pos(0, 0) { }
            explicit ParserError(const std::string& msg, std::pair<int, int> pos) : runtime_error(msg), _pos(pos) { }

            std::pair<int, int> position() const { return _pos; }
        
        private:
            std::pair<int, int> _pos;
        };
        
        static StyleSheet parse(const std::string& cartoCSS);
        
    private:
        static std::pair<int, int> resolvePosition(const std::string& str, std::string::size_type pos);
    };
} }

#endif
