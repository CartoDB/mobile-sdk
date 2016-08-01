/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_COLORGENERATOR_H_
#define _CARTO_MAPNIKVT_COLORGENERATOR_H_

#include <memory>
#include <functional>

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_alternative.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

namespace carto { namespace mvt {
    namespace colorgenimpl {
        namespace phx = boost::phoenix;
        namespace karma = boost::spirit::karma;
        namespace encoding = boost::spirit::iso8859_1;

        template <typename OutputIterator>
        struct Grammar : karma::grammar<OutputIterator, unsigned int(), encoding::space_type> {
            Grammar() : Grammar::base_type(color) {
                using karma::_pass;
                using karma::_val;
                using karma::_1;
                using karma::_2;
                using karma::_3;
                using karma::_4;

                number = 
                      karma::int_   [_pass = phx::bind(&getInteger, _val, _1)]
                    | karma::float_ [_1 = _val]
                    ;

                color =
                      karma::lit("transparent")         [_pass = phx::bind(&isTransparent, _val)]
                    | karma::no_delimit[(karma::lit("rgb") << '(' << number << ',' << number << ',' << number << ')') [_pass = phx::bind(&getRGBColor, _val, _1, _2, _3)]]
                    | karma::no_delimit[(karma::lit("rgba") << '(' << number << ',' << number << ',' << number << ',' << number << ')') [phx::bind(&getRGBAColor, _val, _1, _2, _3, _4)]]
                    ;
            }
                
            karma::rule<OutputIterator, float()> number;
            karma::rule<OutputIterator, unsigned int(), encoding::space_type> color;

        private:
            static bool getInteger(float val, int& intVal) {
                intVal = static_cast<int>(std::floor(val));
                return intVal == val;
            }
            
            static bool isTransparent(unsigned int color) {
                return (color & 0xff000000) == 0;
            }
            
            static bool getRGBColor(unsigned int color, float& r, float &g, float& b) {
                if ((color & 0xff000000) == 0xff000000) {
                    r = static_cast<float>((color >> 16) & 0xff);
                    g = static_cast<float>((color >> 8)  & 0xff);
                    b = static_cast<float>((color >> 0)  & 0xff);
                    return true;
                }
                return false;
            }

            static bool getRGBAColor(unsigned int color, float& r, float &g, float& b, float& a) {
                a = static_cast<float>((color >> 24) & 0xff) / 255.0f;
                r = static_cast<float>((color >> 16) & 0xff) / a;
                g = static_cast<float>((color >> 8)  & 0xff) / a;
                b = static_cast<float>((color >> 0)  & 0xff) / a;
                return true;
            }
        };
    }

    template <typename Iterator> using ColorGenerator = colorgenimpl::Grammar<Iterator>;
} }

#endif
