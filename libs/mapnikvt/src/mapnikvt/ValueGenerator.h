/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_VALUEPARSER_H_
#define _CARTO_MAPNIKVT_VALUEPARSER_H_

#include "Value.h"

#include <memory>
#include <functional>

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_symbols.hpp>
#include <boost/spirit/include/karma_alternative.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

namespace carto { namespace mvt {
    namespace valgenimpl {
        namespace phx = boost::phoenix;
        namespace karma = boost::spirit::karma;
        namespace encoding = boost::spirit::iso8859_1;

        template <typename OutputIterator>
        struct Grammar : karma::grammar<OutputIterator, Value()> {
            Grammar() : Grammar::base_type(value) {
                using karma::_pass;
                using karma::_val;
                using karma::_1;

                esc_char.add('\a', "\\a")('\b', "\\b")('\f', "\\f")('\n', "\\n")
                            ('\r', "\\r")('\t', "\\t")('\v', "\\v")('\\', "\\\\")
                            ('\'', "\\\'")('"', "\\\"");

                string %=
                    '\'' << *(esc_char | karma::print | "\\x" << karma::hex) << '\'';

                value =
                      karma::lit("null")		[_pass = phx::bind(&isNullValue, _val)]
                    | karma::bool_				[_pass = phx::bind(&getBoolValue, _val, _1)]
                    | karma::long_long			[_pass = phx::bind(&getLongValue, _val, _1)]
                    | karma::double_			[_pass = phx::bind(&getDoubleValue, _val, _1)]
                    | string					[_pass = phx::bind(&getStringValue, _val, _1)]
                    ;
            }

            karma::symbols<char, const char *> esc_char;
            karma::rule<OutputIterator, std::string()> string;
            karma::rule<OutputIterator, Value()> value;

        private:
            static bool isNullValue(const Value& val) {
                if (boost::get<boost::blank>(&val)) {
                    return true;
                }
                return false;
            }

            static bool getBoolValue(const Value& val, bool& result) {
                if (auto b = boost::get<bool>(&val)) {
                    result = *b;
                    return true;
                }
                return false;
            }

            static bool getLongValue(const Value& val, long long& result) {
                if (auto i = boost::get<long long>(&val)) {
                    result = *i;
                    return true;
                }
                return false;
            }

            static bool getDoubleValue(const Value& val, double& result) {
                if (auto f = boost::get<double>(&val)) {
                    result = *f;
                    return true;
                }
                return false;
            }

            static bool getStringValue(const Value& val, std::string& result) {
                if (auto s = boost::get<std::string>(&val)) {
                    result = *s;
                    return true;
                }
                return false;
            }
        };
    }

    template <typename Iterator> using ValueGenerator = valgenimpl::Grammar<Iterator>;
} }

#endif
