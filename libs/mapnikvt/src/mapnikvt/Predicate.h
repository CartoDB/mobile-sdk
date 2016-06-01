/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_PREDICATE_H_
#define _CARTO_MAPNIKVT_PREDICATE_H_

#include "Value.h"
#include "ValueConverter.h"
#include "Expression.h"
#include "ExpressionContext.h"

#include <memory>
#include <functional>

namespace carto { namespace mvt {
    class Predicate : public std::enable_shared_from_this<Predicate> {
    public:
        virtual ~Predicate() = default;

        virtual bool evaluate(const ExpressionContext& context) const = 0;

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const = 0;
    };

    class ConstPredicate : public Predicate {
    public:
        explicit ConstPredicate(bool value) : _value(value) { }

        bool getValue() const { return _value; }

        virtual bool evaluate(const ExpressionContext& context) const override {
            return _value;
        }

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override {
        }

    private:
        const bool _value;
    };

    class ExpressionPredicate : public Predicate {
    public:
        explicit ExpressionPredicate(std::shared_ptr<Expression> expr) : _expr(std::move(expr)) { }

        const std::shared_ptr<Expression>& getExpression() const { return _expr; }

        virtual bool evaluate(const ExpressionContext& context) const override {
            return ValueConverter<bool>::convert(_expr->evaluate(context));
        }

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override {
            _expr->fold(fn);
        }

    private:
        const std::shared_ptr<Expression> _expr;
    };

    class ComparisonPredicate : public Predicate {
    public:
        struct Operator {
            virtual ~Operator() = default;

            virtual bool apply(const Value& val1, const Value& val2) const = 0;
        };

        explicit ComparisonPredicate(std::shared_ptr<Operator> op, std::shared_ptr<Expression> expr1, std::shared_ptr<Expression> expr2) : _op(std::move(op)), _expr1(std::move(expr1)), _expr2(std::move(expr2)) { }

        const std::shared_ptr<Operator>& getOperator() const { return _op; }
        const std::shared_ptr<Expression>& getExpression1() const { return _expr1; }
        const std::shared_ptr<Expression>& getExpression2() const { return _expr2; }

        virtual bool evaluate(const ExpressionContext& context) const override {
            return _op->apply(_expr1->evaluate(context), _expr2->evaluate(context));
        }

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override {
            _expr1->fold(fn);
            _expr2->fold(fn);
        }

    protected:
        const std::shared_ptr<Operator> _op;
        const std::shared_ptr<Expression> _expr1;
        const std::shared_ptr<Expression> _expr2;
    };

    class NotPredicate : public Predicate {
    public:
        explicit NotPredicate(std::shared_ptr<Predicate> pred) : _pred(std::move(pred)) { }

        const std::shared_ptr<Predicate>& getPredicate() const { return _pred; }

        virtual bool evaluate(const ExpressionContext& context) const override {
            return !_pred->evaluate(context);
        }

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override {
            _pred->fold(fn);
        }

    protected:
        const std::shared_ptr<Predicate> _pred;
    };

    class OrPredicate : public Predicate {
    public:
        explicit OrPredicate(std::shared_ptr<Predicate> pred1, std::shared_ptr<Predicate> pred2) : _pred1(std::move(pred1)), _pred2(std::move(pred2)) { }

        const std::shared_ptr<Predicate>& getPredicate1() const { return _pred1; }
        const std::shared_ptr<Predicate>& getPredicate2() const { return _pred2; }

        virtual bool evaluate(const ExpressionContext& context) const override {
            return _pred1->evaluate(context) || _pred2->evaluate(context);
        }

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override {
            _pred1->fold(fn);
            _pred2->fold(fn);
        }

    protected:
        const std::shared_ptr<Predicate> _pred1;
        const std::shared_ptr<Predicate> _pred2;
    };

    class AndPredicate : public Predicate {
    public:
        explicit AndPredicate(std::shared_ptr<Predicate> pred1, std::shared_ptr<Predicate> pred2) : _pred1(std::move(pred1)), _pred2(std::move(pred2)) { }

        const std::shared_ptr<Predicate>& getPredicate1() const { return _pred1; }
        const std::shared_ptr<Predicate>& getPredicate2() const { return _pred2; }

        virtual bool evaluate(const ExpressionContext& context) const override {
            return _pred1->evaluate(context) && _pred2->evaluate(context);
        }

        virtual void fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const override {
            _pred1->fold(fn);
            _pred2->fold(fn);
        }

    protected:
        const std::shared_ptr<Predicate> _pred1;
        const std::shared_ptr<Predicate> _pred2;
    };
} }

#endif
