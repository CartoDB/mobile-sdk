/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_COLORPARSER_H_
#define _CARTO_MAPNIKVT_COLORPARSER_H_

#include "CSSColorParser.h"

#include <memory>
#include <functional>
#include <typeinfo>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>

namespace carto { namespace mvt {
    namespace colorparserimpl {
        namespace phx = boost::phoenix;
        namespace qi = boost::spirit::qi;
        namespace repo = boost::spirit::repository::qi;
        namespace encoding = boost::spirit::iso8859_1;

        template <typename Iterator>
        struct Grammar : qi::grammar<Iterator, unsigned int(), encoding::space_type> {
            Grammar() : Grammar::base_type(color) {
                using qi::_val;
                using qi::_pass;
                using qi::_1;
                using qi::_2;
                using qi::_3;
                using qi::_4;

                rgb_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["rgb"]];
                rgba_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["rgba"]];

                number %= qi::float_;

                color =
                      (rgb_kw  >> '(' > number > ',' > number > ',' > number > ')') [_val = phx::bind(&makeRGBColor, _1, _2, _3)]
                    | (rgba_kw >> '(' > number > ',' > number > ',' > number > ',' > number > ')') [_val = phx::bind(&makeRGBAColor, _1, _2, _3, _4)]
                    | qi::lit('#') > (*qi::char_("0-9A-Fa-f"))  [_pass = phx::bind(&getHEXColor, _val, _1)]
                    | (*qi::char_("a-z"))                       [_pass = phx::bind(&getCSSColor, _val, _1)]
                    ;
            }

            qi::rule<Iterator, qi::unused_type()> rgb_kw, rgba_kw;
            qi::rule<Iterator, float()> number;
            qi::rule<Iterator, unsigned int(), encoding::space_type> color;

        private:
            static unsigned int makeRGBColor(float r, float g, float b) {
                float components[3] = { r, g, b };
                unsigned int value = 255;
                for (int i = 0; i < 3; i++) {
                    value = (value << 8) | static_cast<unsigned int>(std::min(255.0f, std::max(0.0f, components[i])));
                }
                return value;
            }

            static unsigned int makeRGBAColor(float r, float g, float b, float a) {
                float alpha = std::min(1.0f, std::max(0.0f, a));
                float components[3] = { r, g, b };
                unsigned int value = static_cast<unsigned int>(alpha * 255.0f);
                for (int i = 0; i < 3; i++) {
                    value = (value << 8) | static_cast<unsigned int>(std::min(255.0f, std::max(0.0f, components[i] * alpha)));
                }
                return value;
            }

            static unsigned int getHEXColor(unsigned int& color, const std::vector<char>& code) {
                unsigned int value = 0;
                if (!parseCSSColor("#" + std::string(code.begin(), code.end()), value)) {
                    return false;
                }
                color = value;
                return true;
            }

            static bool getCSSColor(unsigned int& color, const std::vector<char>& name) {
                unsigned int value = 0;
                if (!parseCSSColor(std::string(name.begin(), name.end()), value)) {
                    return false;
                }
                color = value;
                return true;
            }
        };
    }

    template <typename Iterator> using ColorParser = colorparserimpl::Grammar<Iterator>;
} }

#endif
