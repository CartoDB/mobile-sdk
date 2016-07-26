/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_EXPRESSIONGENERATOR_H_
#define _CARTO_MAPNIKVT_EXPRESSIONGENERATOR_H_

#include "Value.h"
#include "Expression.h"
#include "ExpressionOperator.h"
#include "Predicate.h"
#include "PredicateOperator.h"
#include "ValueGenerator.h"

#include <memory>
#include <functional>

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_alternative.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

namespace carto { namespace mvt {
    namespace exprgenimpl {
        namespace phx = boost::phoenix;
        namespace karma = boost::spirit::karma;
        namespace encoding = boost::spirit::iso8859_1;

        template <typename OutputIterator, bool StringExpression>
        struct Grammar : karma::grammar<OutputIterator, std::shared_ptr<const Expression>()> {
            Grammar() : Grammar::base_type(StringExpression ? stringExpression : genericExpression) {
                using karma::_pass;
                using karma::_val;
                using karma::_1;
                using karma::_2;
                using karma::_3;

                string %=
                    karma::string
                    ;

                stringExpression =
                      string						    [_pass = phx::bind(&getString, _val, _1)]
                    | (stringExpression << stringExpression) [_pass = phx::bind(&getBinaryExpression<ConcatenateOperator>, _val, _1, _2)]
                    | ('[' << stringExpression << ']' ) [_pass = phx::bind(&getVariableExpression, _val, _1)]
                    | ('{' << karma::delimit(encoding::space_type())[expression << '}']) [_1 = _val]
                    ;

                genericExpression =
                    karma::delimit(encoding::space_type())[expression] [_1 = _val]
                    ;

                expression =
                      (term0 << '?' << expression << ':' << expression) [_pass = phx::bind(&getTertiaryExpression<ConditionalOperator>, _val, _1, _2, _3)]
                    | term0								[_1 = _val]
                    ;

                term0 =
                      (term0 << "and" << term1)			[_pass = phx::bind(&getAndPredicate, _val, _1, _2)]
                    | (term0 << "or"  << term1)			[_pass = phx::bind(&getOrPredicate,  _val, _1, _2)]
                    | term1								[_1 = _val]
                    ;

                term1 =
                      (term1 << "<>" << term2)			[_pass = phx::bind(&getComparisonPredicate<NEQOperator>, _val, _1, _2)]
                    | (term1 << "<=" << term2)			[_pass = phx::bind(&getComparisonPredicate<LTEOperator>, _val, _1, _2)]
                    | (term1 << ">=" << term2)			[_pass = phx::bind(&getComparisonPredicate<GTEOperator>, _val, _1, _2)]
                    | (term1 << "!=" << term2)			[_pass = phx::bind(&getComparisonPredicate<NEQOperator>, _val, _1, _2)]
                    | (term1 << '<'  << term2)			[_pass = phx::bind(&getComparisonPredicate<LTOperator>,  _val, _1, _2)]
                    | (term1 << '>'  << term2)			[_pass = phx::bind(&getComparisonPredicate<GTOperator>,  _val, _1, _2)]
                    | (term1 << '='  << term2)			[_pass = phx::bind(&getComparisonPredicate<EQOperator>,  _val, _1, _2)]
                    | term2								[_1 = _val]
                    ;

                term2 =
                      (term2 << '+' << term3)			[_pass = phx::bind(&getBinaryExpression<AddOperator>, _val, _1, _2)]
                    | (term2 << '-' << term3)			[_pass = phx::bind(&getBinaryExpression<SubOperator>, _val, _1, _2)]
                    | term3								[_1 = _val]
                    ;

                term3 =
                      (term3 << '*' << unary)			[_pass = phx::bind(&getBinaryExpression<MulOperator>, _val, _1, _2)]
                    | (term3 << '/' << unary)			[_pass = phx::bind(&getBinaryExpression<DivOperator>, _val, _1, _2)]
                    | (term3 << '%' << unary)			[_pass = phx::bind(&getBinaryExpression<ModOperator>, _val, _1, _2)]
                    | unary								[_1 = _val]
                    ;

                unary =
                      ('-' << unary)					[_pass = phx::bind(&getUnaryExpression<NegOperator>, _val, _1)]
                    | ('!' << unary)					[_pass = phx::bind(&getNotPredicate, _val, _1)]
                    | postfix							[_1 = _val]
                    ;

                postfix =
                      (postfix << '.' << karma::lit("length"))                              [_pass = phx::bind(&getUnaryExpression<LengthOperator>, _val, _1)]
                    | (postfix << '.' << karma::lit("uppercase"))                           [_pass = phx::bind(&getUnaryExpression<UpperCaseOperator>, _val, _1)]
                    | (postfix << '.' << karma::lit("lowercase"))                           [_pass = phx::bind(&getUnaryExpression<LowerCaseOperator>, _val, _1)]
                    | (postfix << '.' << karma::lit("capitalize"))                          [_pass = phx::bind(&getUnaryExpression<CapitalizeOperator>, _val, _1)]
                    | (postfix << '.' << karma::lit("concat")  << '(' << expression << ')') [_pass = phx::bind(&getBinaryExpression<ConcatenateOperator>, _val, _1, _2)]
                    | (postfix << '.' << karma::lit("match")   << '(' << expression << ')') [_pass = phx::bind(&getComparisonPredicate<MatchOperator>, _val, _1, _2)]
                    | (postfix << '.' << karma::lit("replace") << '(' << expression << ',' << expression << ')') [_pass = phx::bind(&getTertiaryExpression<ReplaceOperator>, _val, _1, _2, _3)]
                    | factor							                                    [_1 = _val]
                    ;

                factor =
                      constant							[_pass = phx::bind(&getConstant, _val, _1)]
                    | (karma::lit("step")   << '(' << expression << ',' << (expression % ',') << ')') [_pass = phx::bind(&getInterpolateExpression, InterpolateExpression::Method::STEP, _val, _1, _2)]
                    | (karma::lit("linear") << '(' << expression << ',' << (expression % ',') << ')') [_pass = phx::bind(&getInterpolateExpression, InterpolateExpression::Method::LINEAR, _val, _1, _2)]
                    | (karma::lit("cubic")  << '(' << expression << ',' << (expression % ',') << ')') [_pass = phx::bind(&getInterpolateExpression, InterpolateExpression::Method::CUBIC, _val, _1, _2)]
                    | predicate                         [_pass = phx::bind(&getExpressionPredicate, _val, _1)]
                    | (karma::no_delimit['[' << stringExpression] << ']') [_pass = phx::bind(&getVariableExpression, _val, _1)]
                    | ('(' << expression << ')')		[_1 = _val]
                    ;

                predicate =
                      expression                        [_pass = phx::bind(&getPredicateExpression, _val, _1)]
                    ;
            }

            ValueGenerator<OutputIterator> constant;
            karma::rule<OutputIterator, std::string()> string;
            karma::rule<OutputIterator, std::shared_ptr<const Expression>()> stringExpression, genericExpression;
            karma::rule<OutputIterator, std::shared_ptr<const Expression>(), encoding::space_type> expression, term0, term1, term2, term3, unary, postfix, factor;
            karma::rule<OutputIterator, std::shared_ptr<const Predicate>(), encoding::space_type> predicate;

        private:
            static bool getString(const std::shared_ptr<const Expression>& expr, std::string& str) {
                if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                    Value val = constExpr->getConstant();
                    if (auto s = boost::get<std::string>(&val)) {
                        str = *s;
                        return true;
                    }
                }
                return false;
            }

            static bool getConstant(const std::shared_ptr<const Expression>& expr, Value& val) {
                if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                    val = constExpr->getConstant();
                    return true;
                }
                return false;
            }

            static bool getVariableExpression(const std::shared_ptr<const Expression>& expr, std::shared_ptr<const Expression>& expr1) {
                if (auto varExpr = std::dynamic_pointer_cast<const VariableExpression>(expr)) {
                    expr1 = varExpr->getVariableExpression();
                    return true;
                }
                return false;
            }

            static bool getExpressionPredicate(const std::shared_ptr<const Expression>& expr, std::shared_ptr<const Predicate>& pred1) {
                if (auto predExpr = std::dynamic_pointer_cast<const PredicateExpression>(expr)) {
                    pred1 = predExpr->getPredicate();
                    return true;
                }
                return false;
            }

            static bool getPredicateExpression(const std::shared_ptr<const Predicate>& pred, std::shared_ptr<const Expression>& expr1) {
                if (auto exprPred = std::dynamic_pointer_cast<const ExpressionPredicate>(pred)) {
                    expr1 = exprPred->getExpression();
                    return true;
                }
                return false;
            }

            static bool getNotPredicate(const std::shared_ptr<const Expression>& expr, std::shared_ptr<const Expression>& expr1) {
                if (auto predExpr = std::dynamic_pointer_cast<const PredicateExpression>(expr)) {
                    if (auto notPred = std::dynamic_pointer_cast<const NotPredicate>(predExpr->getPredicate())) {
                        expr1 = std::make_shared<PredicateExpression>(notPred->getPredicate());
                        return true;
                    }
                }
                return false;
            }

            static bool getOrPredicate(const std::shared_ptr<const Expression>& expr, std::shared_ptr<const Expression>& expr1, std::shared_ptr<const Expression>& expr2) {
                if (auto predExpr = std::dynamic_pointer_cast<const PredicateExpression>(expr)) {
                    if (auto orPred = std::dynamic_pointer_cast<const OrPredicate>(predExpr->getPredicate())) {
                        expr1 = std::make_shared<PredicateExpression>(orPred->getPredicate1());
                        expr2 = std::make_shared<PredicateExpression>(orPred->getPredicate2());
                        return true;
                    }
                }
                return false;
            }

            static bool getAndPredicate(const std::shared_ptr<const Expression>& expr, std::shared_ptr<const Expression>& expr1, std::shared_ptr<const Expression>& expr2) {
                if (auto predExpr = std::dynamic_pointer_cast<const PredicateExpression>(expr)) {
                    if (auto andPred = std::dynamic_pointer_cast<const AndPredicate>(predExpr->getPredicate())) {
                        expr1 = std::make_shared<PredicateExpression>(andPred->getPredicate1());
                        expr2 = std::make_shared<PredicateExpression>(andPred->getPredicate2());
                        return true;
                    }
                }
                return false;
            }

            template <typename Op>
            static bool getComparisonPredicate(const std::shared_ptr<const Expression>& expr, std::shared_ptr<const Expression>& expr1, std::shared_ptr<const Expression>& expr2) {
                if (auto predExpr = std::dynamic_pointer_cast<const PredicateExpression>(expr)) {
                    if (auto comparisonPred = std::dynamic_pointer_cast<const ComparisonPredicate>(predExpr->getPredicate())) {
                        if (std::dynamic_pointer_cast<const Op>(comparisonPred->getOperator())) {
                            expr1 = comparisonPred->getExpression1();
                            expr2 = comparisonPred->getExpression2();
                            return true;
                        }
                    }
                }
                return false;
            }

            template <typename Op>
            static bool getUnaryExpression(const std::shared_ptr<const Expression>& expr, std::shared_ptr<const Expression>& expr1) {
                if (auto unaryExpr = std::dynamic_pointer_cast<const UnaryExpression>(expr)) {
                    if (std::dynamic_pointer_cast<const Op>(unaryExpr->getOperator())) {
                        expr1 = unaryExpr->getExpression();
                        return true;
                    }
                }
                return false;
            }

            template <typename Op>
            static bool getBinaryExpression(const std::shared_ptr<const Expression>& expr, std::shared_ptr<const Expression>& expr1, std::shared_ptr<const Expression>& expr2) {
                if (auto binaryExpr = std::dynamic_pointer_cast<const BinaryExpression>(expr)) {
                    if (std::dynamic_pointer_cast<const Op>(binaryExpr->getOperator())) {
                        expr1 = binaryExpr->getExpression1();
                        expr2 = binaryExpr->getExpression2();
                        return true;
                    }
                }
                return false;
            }

            template <typename Op>
            static bool getTertiaryExpression(const std::shared_ptr<const Expression>& expr, std::shared_ptr<const Expression>& expr1, std::shared_ptr<const Expression>& expr2, std::shared_ptr<const Expression>& expr3) {
                if (auto tertiaryExpr = std::dynamic_pointer_cast<const TertiaryExpression>(expr)) {
                    if (std::dynamic_pointer_cast<const Op>(tertiaryExpr->getOperator())) {
                        expr1 = tertiaryExpr->getExpression1();
                        expr2 = tertiaryExpr->getExpression2();
                        expr3 = tertiaryExpr->getExpression3();
                        return true;
                    }
                }
                return false;
            }

            static bool getInterpolateExpression(InterpolateExpression::Method method, const std::shared_ptr<const Expression>& expr, std::shared_ptr<const Expression>& expr1, std::vector<std::shared_ptr<const Expression>>& exprs) {
                if (auto interpolateExpr = std::dynamic_pointer_cast<const InterpolateExpression>(expr)) {
                    if (interpolateExpr->getMethod() == method) {
                        expr1 = interpolateExpr->getTimeExpression();
                        exprs = interpolateExpr->getKeyFrameExpressions();
                        return true;
                    }
                }
                return false;
            }
        };
    }

    template <typename Iterator> using ExpressionGenerator = exprgenimpl::Grammar<Iterator, false>;
    template <typename Iterator> using StringExpressionGenerator = exprgenimpl::Grammar<Iterator, true>;
} }

#endif
