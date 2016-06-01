/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_EXPRESSION_H_
#define _CARTO_MAPNIKVT_EXPRESSION_H_

#include "Value.h"
#include "ValueConverter.h"
#include "ExpressionContext.h"

#include <memory>
#include <functional>

namespace carto { namespace mvt {
    class Predicate;
    
    class Expression : public std::enable_shared_from_this<Expression> {
    public:
        virtual ~Expression() = default;

        virtual Value evaluate(const ExpressionContext& context) const = 0;

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const = 0;
    };

    class ConstExpression : public Expression {
    public:
        explicit ConstExpression(Value constant) : _constant(std::move(constant)) { }

        const Value& getConstant() const { return _constant; }

        virtual Value evaluate(const ExpressionContext& context) const override {
            return _constant;
        }

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override {
            fn(shared_from_this());
        }

    private:
        const Value _constant;
    };

    class VariableExpression : public Expression {
    public:
        explicit VariableExpression(std::string variableName) : _variableExpr(std::make_shared<ConstExpression>(Value(std::move(variableName)))) { }
        explicit VariableExpression(std::shared_ptr<Expression> variableExpr) : _variableExpr(std::move(variableExpr)) { }

        const std::shared_ptr<Expression>& getVariableExpression() const { return _variableExpr; }
        std::string getVariableName(const ExpressionContext& context) const { return ValueConverter<std::string>::convert(_variableExpr->evaluate(context)); }

        virtual Value evaluate(const ExpressionContext& context) const override {
            return context.getVariable(getVariableName(context));
        }

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override {
            fn(shared_from_this());
            _variableExpr->fold(fn);
        }
    
    private:
        const std::shared_ptr<Expression> _variableExpr;
    };

    class PredicateExpression : public Expression {
    public:
        explicit PredicateExpression(std::shared_ptr<Predicate> pred) : _pred(std::move(pred)) { }

        const std::shared_ptr<Predicate>& getPredicate() const { return _pred; }

        virtual Value evaluate(const ExpressionContext& context) const override;

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override;

    private:
        const std::shared_ptr<Predicate> _pred;
    };

    class UnaryExpression : public Expression {
    public:
        struct Operator {
            virtual ~Operator() = default;

            virtual Value apply(const Value& val) const = 0;
        };

        explicit UnaryExpression(std::shared_ptr<Operator> op, std::shared_ptr<Expression> expr) : _op(std::move(op)), _expr(std::move(expr)) { }

        const std::shared_ptr<Operator>& getOperator() const { return _op; }
        const std::shared_ptr<Expression>& getExpression() const { return _expr; }

        virtual Value evaluate(const ExpressionContext& context) const override {
            return _op->apply(_expr->evaluate(context));
        }

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override {
            fn(shared_from_this());
            _expr->fold(fn);
        }
    
    private:
        const std::shared_ptr<Operator> _op;
        const std::shared_ptr<Expression> _expr;
    };

    class BinaryExpression : public Expression {
    public:
        struct Operator {
            virtual ~Operator() = default;

            virtual Value apply(const Value& val1, const Value& val2) const = 0;
        };

        explicit BinaryExpression(std::shared_ptr<Operator> op, std::shared_ptr<Expression> expr1, std::shared_ptr<Expression> expr2) : _op(std::move(op)), _expr1(std::move(expr1)), _expr2(std::move(expr2)) { }

        const std::shared_ptr<Operator>& getOperator() const { return _op; }
        const std::shared_ptr<Expression>& getExpression1() const { return _expr1; }
        const std::shared_ptr<Expression>& getExpression2() const { return _expr2; }

        virtual Value evaluate(const ExpressionContext& context) const override {
            return _op->apply(_expr1->evaluate(context), _expr2->evaluate(context));
        }

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override {
            fn(shared_from_this());
            _expr1->fold(fn);
            _expr2->fold(fn);
        }

    protected:
        const std::shared_ptr<Operator> _op;
        const std::shared_ptr<Expression> _expr1;
        const std::shared_ptr<Expression> _expr2;
    };

    class TertiaryExpression : public Expression {
    public:
        struct Operator {
            virtual ~Operator() = default;

            virtual Value apply(const Value& val1, const Value& val2, const Value& val3) const = 0;
        };

        explicit TertiaryExpression(std::shared_ptr<Operator> op, std::shared_ptr<Expression> expr1, std::shared_ptr<Expression> expr2, std::shared_ptr<Expression> expr3) : _op(std::move(op)), _expr1(std::move(expr1)), _expr2(std::move(expr2)), _expr3(std::move(expr3)) { }

        const std::shared_ptr<Operator>& getOperator() const { return _op; }
        const std::shared_ptr<Expression>& getExpression1() const { return _expr1; }
        const std::shared_ptr<Expression>& getExpression2() const { return _expr2; }
        const std::shared_ptr<Expression>& getExpression3() const { return _expr3; }

        virtual Value evaluate(const ExpressionContext& context) const override {
            return _op->apply(_expr1->evaluate(context), _expr2->evaluate(context), _expr3->evaluate(context));
        }

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override {
            fn(shared_from_this());
            _expr1->fold(fn);
            _expr2->fold(fn);
            _expr3->fold(fn);
        }
    
    private:
        const std::shared_ptr<Operator> _op;
        const std::shared_ptr<Expression> _expr1;
        const std::shared_ptr<Expression> _expr2;
        const std::shared_ptr<Expression> _expr3;
    };
} }

#endif
