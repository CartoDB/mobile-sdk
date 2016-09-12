/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_STYLESELECTOREXPRESSIONIMPL_H_
#define _CARTO_STYLESELECTOREXPRESSIONIMPL_H_

#ifdef _CARTO_GDAL_SUPPORT

#include "styles/StyleSelectorContext.h"
#include "styles/StyleSelectorExpression.h"

#include <limits>

#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>

namespace carto {
    namespace StyleSelectorExpressionImpl {
        enum {
            NULL_VALUE = 0, DOUBLE_VALUE = 1, STRING_VALUE = 2
        };

        using Value = boost::variant<boost::blank, double, std::string>;

        using Expression = StyleSelectorExpression;

        using Context = StyleSelectorContext;

        struct IsNullPredicate {
            bool operator() (const Value& val) const { return val.which() == NULL_VALUE; }
        };

        struct IsNotNullPredicate {
            bool operator() (const Value& val) const { return val.which() != NULL_VALUE; }
        };

        template <template <typename T> class Op>
        struct ComparisonPredicate {
            bool operator() (const Value& val1, const Value& val2) const {
                if (val1.which() == NULL_VALUE || val2.which() == NULL_VALUE) {
                    return false;
                }
                if (val1.which() == STRING_VALUE && val2.which() == STRING_VALUE) {
                    return Op<std::string>()(boost::get<std::string>(val1), boost::get<std::string>(val2));
                }
                if (val1.which() == DOUBLE_VALUE && val2.which() == DOUBLE_VALUE) {
                    return Op<double>()(boost::get<double>(val1), boost::get<double>(val2));
                }
                double num1 = std::numeric_limits<double>::quiet_NaN();
                try {
                    num1 = (val1.which() == STRING_VALUE ? boost::lexical_cast<double>(val1) : boost::get<double>(val1));
                } catch (const boost::bad_lexical_cast&) { }
                double num2 = std::numeric_limits<double>::quiet_NaN();
                try {
                    num2 = (val2.which() == STRING_VALUE ? boost::lexical_cast<double>(val2) : boost::get<double>(val2));
                } catch (const boost::bad_lexical_cast&) { }
                return Op<double>()(num1, num2);
            }
        };

        using EqPredicate = ComparisonPredicate<std::equal_to>;
        using NeqPredicate = ComparisonPredicate<std::not_equal_to>;
        using GtPredicate = ComparisonPredicate<std::greater>;
        using GtePredicate = ComparisonPredicate<std::greater_equal>;
        using LtPredicate = ComparisonPredicate<std::less>;
        using LtePredicate = ComparisonPredicate<std::less_equal>;

        struct Operand {
            virtual ~Operand() = default;
            virtual Value evaluate(const Context& context) const = 0;
        };

        struct ConstOperand : public Operand {
            ConstOperand(const Value& value) : _value(value) { }
            virtual Value evaluate(const Context& context) const { return _value; }
            static std::shared_ptr<ConstOperand> create(const Value& value) { return std::make_shared<ConstOperand>(value); }
        private:
            Value _value;
        };

        struct VariableOperand : public Operand {
            VariableOperand(const std::string& name) : _name(name) { }
            virtual Value evaluate(const Context& context) const { boost::variant<double, std::string> value; if (!context.getVariable(_name, value)) return Value(); return Value(value); }
            static std::shared_ptr<VariableOperand> create(const std::string& name) { return std::make_shared<VariableOperand>(name); }
        private:
            std::string _name;
        };

        struct NotExpression : public Expression {
            NotExpression(const std::shared_ptr<Expression>& expr) : _expr(expr) { }
            virtual bool evaluate(const Context& context) const { return !_expr->evaluate(context); }
            static std::shared_ptr<NotExpression> create(const std::shared_ptr<Expression>& expr) { return std::make_shared<NotExpression>(expr); }
        private:
            std::shared_ptr<Expression> _expr;
        };

        struct OrExpression : public Expression {
            OrExpression(const std::shared_ptr<Expression>& expr1, const std::shared_ptr<Expression>& expr2) : _expr1(expr1), _expr2(expr2) { }
            virtual bool evaluate(const Context& context) const { return _expr1->evaluate(context) || _expr2->evaluate(context); }
            static std::shared_ptr<OrExpression> create(const std::shared_ptr<Expression>& expr1, const std::shared_ptr<Expression>& expr2) { return std::make_shared<OrExpression>(expr1, expr2); }
        private:
            std::shared_ptr<Expression> _expr1, _expr2;
        };

        struct AndExpression : public Expression {
            AndExpression(const std::shared_ptr<Expression>& expr1, const std::shared_ptr<Expression>& expr2) : _expr1(expr1), _expr2(expr2) { }
            virtual bool evaluate(const Context& context) const { return _expr1->evaluate(context) && _expr2->evaluate(context); }
            static std::shared_ptr<AndExpression> create(const std::shared_ptr<Expression>& expr1, const std::shared_ptr<Expression>& expr2) { return std::make_shared<AndExpression>(expr1, expr2); }
        private:
            std::shared_ptr<Expression> _expr1, _expr2;
        };

        template <typename Pred>
        struct UnaryPredicateExpression : public Expression {
            UnaryPredicateExpression(const std::shared_ptr<Pred>& pred, const std::shared_ptr<Operand>& op) : _pred(pred), _op(op) { }
            virtual bool evaluate(const Context& context) const { return (*_pred)(_op->evaluate(context)); }
            static std::shared_ptr<UnaryPredicateExpression> create(const std::shared_ptr<Operand>& op) { return std::make_shared<UnaryPredicateExpression>(std::make_shared<Pred>(), op); }
        private:
            std::shared_ptr<Pred> _pred;
            std::shared_ptr<Operand> _op;
        };

        template <typename Pred>
        struct BinaryPredicateExpression : public Expression {
            BinaryPredicateExpression(const std::shared_ptr<Pred>& pred, const std::shared_ptr<Operand>& op1, const std::shared_ptr<Operand>& op2) : _pred(pred), _op1(op1), _op2(op2) { }
            virtual bool evaluate(const Context& context) const { return (*_pred)(_op1->evaluate(context), _op2->evaluate(context)); }
            static std::shared_ptr<BinaryPredicateExpression> create(const std::shared_ptr<Operand>& op1, const std::shared_ptr<Operand>& op2) { return std::make_shared<BinaryPredicateExpression>(std::make_shared<Pred>(), op1, op2); }
        private:
            std::shared_ptr<Pred> _pred;
            std::shared_ptr<Operand> _op1, _op2;
        };
    }
}

#endif

#endif
