/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_EXPRESSIONPARSER_H_
#define _CARTO_MAPNIKVT_EXPRESSIONPARSER_H_

#include "Value.h"
#include "Expression.h"
#include "ExpressionOperator.h"
#include "Predicate.h"
#include "PredicateOperator.h"
#include "ValueParser.h"

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
    namespace exprparserimpl {
        namespace phx = boost::phoenix;
        namespace qi = boost::spirit::qi;
        namespace repo = boost::spirit::repository::qi;
        namespace encoding = boost::spirit::iso8859_1;

        template <typename Iterator, bool StringExpression>
        struct Grammar : qi::grammar<Iterator, std::shared_ptr<Expression>()> {
            Grammar() : Grammar::base_type(StringExpression ? stringExpression : genericExpression) {
                using qi::_val;
                using qi::_1;
                using qi::_2;

                le_kw  = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["le"]];
                ge_kw  = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["ge"]];
                lt_kw  = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["lt"]];
                gt_kw  = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["gt"]];
                eq_kw  = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["eq"]];
                neq_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["neq"]];
                or_kw  = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["or"]];
                and_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["and"]];
                not_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["not"]];
                length_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["length"]];
                uppercase_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["uppercase"]];
                lowercase_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["lowercase"]];
                capitalize_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["capitalize"]];
                concat_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["concat"]];
                match_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["match"]];
                replace_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["replace"]];
                step_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["step"]];
                linear_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["linear"]];
                cubic_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["cubic"]];

                string %=
                    qi::lexeme[+(qi::print - '[' - ']' - '{' - '}')]
                    ;

                stringExpression =
                    ( (string                                        [_val = phx::bind(&makeStringExpression, _1)])
                    | ('[' > stringExpression  > ']')	             [_val = phx::bind(&makeVariableExpression, _1)]
            		| ('{' > qi::skip(encoding::space_type())[expression > '}']) [_val = _1]
                    )
                    > -(stringExpression                             [_val = phx::bind(&makeBinaryExpression<ConcatenateOperator>, _val, _1)])
                    ;

                genericExpression =
                    qi::skip(encoding::space_type())[expression]     [_val = _1]
                    ;

                expression =
                    term0											 [_val = _1]
                    >> -( (qi::lit('?') > expression > ':' > expression) [_val = phx::bind(&makeTertiaryExpression<ConditionalOperator>, _val, _1, _2)]
                        )
                    ;

                term0 =
                    term1											 [_val = _1]
                    >> *( ((qi::lit("&&") | and_kw) > term1)		 [_val = phx::bind(&makeAndPredicate, _val, _1)]
                        | ((qi::lit("||") | or_kw) > term1)		     [_val = phx::bind(&makeOrPredicate,  _val, _1)]
                        )
                    ;

                term1 =
                    term2											 [_val = _1]
                    >> *( ((qi::lit("<>") | "!=" | neq_kw) > term2)  [_val = phx::bind(&makeComparisonPredicate<NEQOperator>, _val, _1)]
                        | ((qi::lit("<=") | le_kw        ) > term2)  [_val = phx::bind(&makeComparisonPredicate<LTEOperator>, _val, _1)]
                        | ((qi::lit(">=") | ge_kw        ) > term2)  [_val = phx::bind(&makeComparisonPredicate<GTEOperator>, _val, _1)]
                        | ((qi::lit('<' ) | lt_kw        ) > term2)  [_val = phx::bind(&makeComparisonPredicate<LTOperator>,  _val, _1)]
                        | ((qi::lit('>' ) | gt_kw        ) > term2)  [_val = phx::bind(&makeComparisonPredicate<GTOperator>,  _val, _1)]
                        | ((qi::lit('=' ) | eq_kw        ) > term2)  [_val = phx::bind(&makeComparisonPredicate<EQOperator>,  _val, _1)]
                        )
                    ;

                term2 =
                    term3											  [_val = _1]
                    >> *( (qi::lit("+") > term3)					  [_val = phx::bind(&makeBinaryExpression<AddOperator>, _val, _1)]
                        | (qi::lit("-") > term3)					  [_val = phx::bind(&makeBinaryExpression<SubOperator>, _val, _1)]
                        )
                    ;

                term3 =
                    unary											  [_val = _1]
                    >> *( (qi::lit("*") > unary)					  [_val = phx::bind(&makeBinaryExpression<MulOperator>, _val, _1)]
                        | (qi::lit("/") > unary)					  [_val = phx::bind(&makeBinaryExpression<DivOperator>, _val, _1)]
                        | (qi::lit("%") > unary)					  [_val = phx::bind(&makeBinaryExpression<ModOperator>, _val, _1)]
                        )
                    ;

                unary =
                        postfix										  [_val = _1]
                    |  (qi::lit('-')            > unary)			  [_val = phx::bind(&makeUnaryExpression<NegOperator>, _1)]
                    | ((qi::lit('!') || not_kw) > unary)			  [_val = phx::bind(&makeNotPredicate, _1)]
                    ;

                postfix =
                    factor											  [_val = _1]
                    >> *('.' >> ( length_kw                           [_val = phx::bind(&makeUnaryExpression<LengthOperator>, _val)]
                                | uppercase_kw                        [_val = phx::bind(&makeUnaryExpression<UpperCaseOperator>, _val)]
                                | lowercase_kw                        [_val = phx::bind(&makeUnaryExpression<LowerCaseOperator>, _val)]
                                | capitalize_kw                       [_val = phx::bind(&makeUnaryExpression<CapitalizeOperator>, _val)]
                                | (concat_kw  >> ('(' > expression > ')')) [_val = phx::bind(&makeBinaryExpression<ConcatenateOperator>, _val, _1)]
                                | (match_kw >> ('(' > expression > ')')) [_val = phx::bind(&makeComparisonPredicate<MatchOperator>, _val, _1)]
                                | (replace_kw >> ('(' > expression > ',' > expression > ')')) [_val = phx::bind(&makeTertiaryExpression<ReplaceOperator>, _val, _1, _2)]
                                )
                        )
                    ;

                factor =
                      constant										  [_val = phx::bind(&makeConstExpression, _1)]
                    | (step_kw   >> '(' > expression > ',' > (constant % ',') > ')') [_val = phx::bind(&makeInterpolateExpression, InterpolateExpression::Method::STEP, _1, _2)]
                    | (linear_kw >> '(' > expression > ',' > (constant % ',') > ')') [_val = phx::bind(&makeInterpolateExpression, InterpolateExpression::Method::LINEAR, _1, _2)]
                    | (cubic_kw  >> '(' > expression > ',' > (constant % ',') > ')') [_val = phx::bind(&makeInterpolateExpression, InterpolateExpression::Method::CUBIC, _1, _2)]
                    | ('[' > stringExpression > ']')				  [_val = phx::bind(&makeVariableExpression, _1)]
                    | ('(' > expression > ')')				    	  [_val = _1]
                    ;
            }

            ValueParser<Iterator> constant;
            qi::rule<Iterator, std::string()> string;
            qi::rule<Iterator, qi::unused_type()> not_kw, and_kw, or_kw, neq_kw, eq_kw, le_kw, ge_kw, lt_kw, gt_kw, length_kw, uppercase_kw, lowercase_kw, capitalize_kw, concat_kw, match_kw, replace_kw, step_kw, linear_kw, cubic_kw;
            qi::rule<Iterator, std::shared_ptr<Expression>()> stringExpression, genericExpression;
            qi::rule<Iterator, std::shared_ptr<Expression>(), encoding::space_type> expression, term0, term1, term2, term3, unary, postfix, factor;

        private:
            static std::shared_ptr<Expression> makeStringExpression(std::string str) {
                return std::make_shared<ConstExpression>(Value(std::move(str)));
            }

            static std::shared_ptr<Expression> makeConstExpression(Value val) {
                return std::make_shared<ConstExpression>(std::move(val));
            }

            static std::shared_ptr<Expression> makeVariableExpression(std::shared_ptr<const Expression> expr) {
                return std::make_shared<VariableExpression>(std::move(expr));
            }

            static std::shared_ptr<Expression> makeNotPredicate(std::shared_ptr<const Expression> expr) {
                std::shared_ptr<const Predicate> exprPred;
                if (auto predExpr = std::dynamic_pointer_cast<const PredicateExpression>(expr)) {
                    exprPred = predExpr->getPredicate();
                } else {
                    exprPred = std::make_shared<ExpressionPredicate>(expr);
                }
                return std::make_shared<PredicateExpression>(std::make_shared<NotPredicate>(exprPred));
            }

            static std::shared_ptr<Expression> makeOrPredicate(std::shared_ptr<const Expression> expr1, std::shared_ptr<const Expression> expr2) {
                std::shared_ptr<const Predicate> exprPred1, exprPred2;
                if (auto predExpr1 = std::dynamic_pointer_cast<const PredicateExpression>(expr1)) {
                    exprPred1 = predExpr1->getPredicate();
                }
                else {
                    exprPred1 = std::make_shared<ExpressionPredicate>(expr1);
                }
                if (auto predExpr2 = std::dynamic_pointer_cast<const PredicateExpression>(expr2)) {
                    exprPred2 = predExpr2->getPredicate();
                }
                else {
                    exprPred2 = std::make_shared<ExpressionPredicate>(expr2);
                }
                return std::make_shared<PredicateExpression>(std::make_shared<OrPredicate>(exprPred1, exprPred2));
            }

            static std::shared_ptr<Expression> makeAndPredicate(std::shared_ptr<const Expression> expr1, std::shared_ptr<const Expression> expr2) {
                std::shared_ptr<const Predicate> exprPred1, exprPred2;
                if (auto predExpr1 = std::dynamic_pointer_cast<const PredicateExpression>(expr1)) {
                    exprPred1 = predExpr1->getPredicate();
                }
                else {
                    exprPred1 = std::make_shared<ExpressionPredicate>(expr1);
                }
                if (auto predExpr2 = std::dynamic_pointer_cast<const PredicateExpression>(expr2)) {
                    exprPred2 = predExpr2->getPredicate();
                }
                else {
                    exprPred2 = std::make_shared<ExpressionPredicate>(expr2);
                }
                return std::make_shared<PredicateExpression>(std::make_shared<AndPredicate>(exprPred1, exprPred2));
            }

            template <typename Op>
            static std::shared_ptr<Expression> makeComparisonPredicate(std::shared_ptr<const Expression> expr1, std::shared_ptr<const Expression> expr2) {
                return std::make_shared<PredicateExpression>(std::make_shared<ComparisonPredicate>(std::make_shared<Op>(), std::move(expr1), std::move(expr2)));
            }

            template <typename Op>
            static std::shared_ptr<Expression> makeUnaryExpression(std::shared_ptr<const Expression> expr) {
                return std::make_shared<UnaryExpression>(std::make_shared<Op>(), std::move(expr));
            }

            template <typename Op>
            static std::shared_ptr<Expression> makeBinaryExpression(std::shared_ptr<const Expression> expr1, std::shared_ptr<const Expression> expr2) {
                return std::make_shared<BinaryExpression>(std::make_shared<Op>(), std::move(expr1), std::move(expr2));
            }

            template <typename Op>
            static std::shared_ptr<Expression> makeTertiaryExpression(std::shared_ptr<const Expression> expr1, std::shared_ptr<const Expression> expr2, std::shared_ptr<const Expression> expr3) {
                return std::make_shared<TertiaryExpression>(std::make_shared<Op>(), std::move(expr1), std::move(expr2), std::move(expr3));
            }

            static std::shared_ptr<Expression> makeInterpolateExpression(InterpolateExpression::Method method, std::shared_ptr<const Expression> timeExpr, std::vector<Value> keyFrames) {
                return std::make_shared<InterpolateExpression>(method, timeExpr, keyFrames);
            }
        };
    }

    template <typename Iterator> using ExpressionParser = exprparserimpl::Grammar<Iterator, false>;
    template <typename Iterator> using StringExpressionParser = exprparserimpl::Grammar<Iterator, true>;
} }

#endif
