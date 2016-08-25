/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_STYLESELECTOREXPRESSIONPARSER_H_
#define _CARTO_STYLESELECTOREXPRESSIONPARSER_H_

#ifdef _CARTO_GDAL_SUPPORT

#include "styles/StyleSelectorExpression.h"
#include "styles/StyleSelectorExpressionImpl.h"

#include <memory>
#include <functional>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace carto {
    namespace StyleSelectorExpressionImpl {
        namespace phx = boost::phoenix;
        namespace qi = boost::spirit::qi;
        namespace repo = boost::spirit::repository::qi;
        namespace encoding = boost::spirit::iso8859_1;
    
        template <typename Iterator>
        struct Grammar : qi::grammar<Iterator, std::shared_ptr<StyleSelectorExpression>(), encoding::space_type> {
            Grammar() : Grammar::base_type(expression) {
                using qi::_val;
                using qi::_1;
                using qi::_2;

                unesc_char.add("\\a", '\a')("\\b", '\b')("\\f", '\f')("\\n", '\n')
                ("\\r", '\r')("\\t", '\t')("\\v", '\v')("\\\\", '\\')
                ("\\\'", '\'')("\\\"", '\"');

                is_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["is"]];
                or_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["or"]];
                and_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["and"]];
                not_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["not"]];
                null_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["null"]];

                string =
                    '\'' >> *(unesc_char | "\\x" >> qi::hex | (qi::print - '\'')) >> '\'';

                value =
                      null_kw     [_val = phx::construct<Value>()]
                    | qi::double_ [_val = phx::construct<Value>(_1)]
                    | string      [_val = phx::construct<Value>(_1)]
                    ;

                identifier %=
                    qi::lexeme[qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z0-9_")] >> -(qi::string("::") >> identifier)
                    ;

                expression =
                    term1 [_val = _1] >> *((("||" | or_kw) >> term1) [_val = phx::bind(&OrExpression::create, _val, _1)])
                    ;

                term1 =
                    term2 [_val = _1] >> *((("&&" | and_kw) >> term1) [_val = phx::bind(&AndExpression::create, _val, _1)])
                    ;

                term2 =
                      term3 [_val = _1]
                    | (("!" || not_kw) >> term3) [_val = phx::bind(&NotExpression::create, _1)]
                    ;

                term3 =
                      predicate [_val = _1]
                    | ('(' >> expression >> ')') [_val = _1]
                    ;

                predicate =
                      (operand >> "<=" >> operand) [_val = phx::bind(&BinaryPredicateExpression<LtePredicate>::create, _1, _2)]
                    | (operand >> ">=" >> operand) [_val = phx::bind(&BinaryPredicateExpression<GtePredicate>::create, _1, _2)]
                    | (operand >> "<"  >> operand) [_val = phx::bind(&BinaryPredicateExpression<LtPredicate>::create, _1, _2)]
                    | (operand >> ">"  >> operand) [_val = phx::bind(&BinaryPredicateExpression<GtPredicate>::create, _1, _2)]
                    | (operand >> (qi::lit("==") | qi::lit("=") ) >> operand) [_val = phx::bind(&BinaryPredicateExpression<EqPredicate>::create, _1, _2)]
                    | (operand >> (qi::lit("<>") | qi::lit("!=")) >> operand) [_val = phx::bind(&BinaryPredicateExpression<NeqPredicate>::create, _1, _2)]
                    | (operand >> is_kw >> null_kw)           [_val = phx::bind(&UnaryPredicateExpression<IsNullPredicate>::create, _1)]
                    | (operand >> is_kw >> not_kw >> null_kw) [_val = phx::bind(&UnaryPredicateExpression<IsNotNullPredicate>::create, _1)]
                    ;
        
                operand =
                      value [_val = phx::bind(&ConstOperand::create, _1)]
                    | identifier [_val = phx::bind(&VariableOperand::create, _1)]
                    ;
            }

            qi::symbols<char const, char const> unesc_char;
            qi::rule<Iterator, qi::unused_type()> is_kw, or_kw, and_kw, not_kw, null_kw;
            qi::rule<Iterator, std::string()> string;
            qi::rule<Iterator, Value()> value;
            qi::rule<Iterator, std::string()> identifier;
            qi::rule<Iterator, std::shared_ptr<Expression>(), encoding::space_type> expression, term1, term2, term3, predicate;
            qi::rule<Iterator, std::shared_ptr<Operand>(), encoding::space_type> operand;
        };
    }

    template <typename Iterator> using StyleSelectorExpressionParser = StyleSelectorExpressionImpl::Grammar<Iterator>;
}

#endif

#endif
