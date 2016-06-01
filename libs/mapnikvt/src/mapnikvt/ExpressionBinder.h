/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_EXPRESSIONBINDER_H_
#define _CARTO_MAPNIKVT_EXPRESSIONBINDER_H_

#include "Expression.h"
#include "ExpressionContext.h"
#include "ValueConverter.h"
#include "vt/Color.h"

#include <vector>
#include <memory>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

#include <cglib/mat.h>

namespace carto { namespace mvt {
    namespace exprbinderimpl {
        struct Setter : public boost::static_visitor<> {
            explicit Setter(const ExpressionContext& context) : _exprContext(context) { }

            template <typename V>
            void operator()(V & binding) const {
                Value val = binding.expr->evaluate(_exprContext);
                if (binding.object) {
                    *binding.field = (binding.object->*binding.memberConvertFn)(val);
                }
                else {
                    *binding.field = (*binding.convertFn)(val);
                }
            }

        private:
            const ExpressionContext& _exprContext;
        };
    }

    template <typename T>
    class ExpressionBinder {
    public:
        ExpressionBinder() = default;

        template <typename V>
        ExpressionBinder& bind(V* field, const std::shared_ptr<Expression>& expr) {
            if (auto constExpr = std::dynamic_pointer_cast<ConstExpression>(expr)) {
                *field = ValueConverter<V>::convert(constExpr->getConstant());
            }
            else {
                _exprs.push_back(expr);
                _bindingMap.insert({ field, Binding<V>(field, expr, &ValueConverter<V>::convert) });
            }
            return *this;
        }

        template <typename V>
        ExpressionBinder& bind(V* field, const std::shared_ptr<Expression>& expr, V(*convertFn)(const Value& val)) {
            if (auto constExpr = std::dynamic_pointer_cast<ConstExpression>(expr)) {
                *field = (*convertFn)(constExpr->getConstant());
            }
            else {
                _exprs.push_back(expr);
                _bindingMap.insert({ field, Binding<V>(field, expr, convertFn) });
            }
            return *this;
        }

        template <typename V>
        ExpressionBinder& bind(V* field, const std::shared_ptr<Expression>& expr, V (T::*memberConvertFn)(const Value& val) const, const T* object) {
            if (auto constExpr = std::dynamic_pointer_cast<ConstExpression>(expr)) {
                *field = (object->*memberConvertFn)(constExpr->getConstant());
            }
            else {
                _exprs.push_back(expr);
                _bindingMap.insert({ field, Binding<V>(field, expr, memberConvertFn, object) });
            }
            return *this;
        }

        const std::vector<std::shared_ptr<Expression>>& getExpressions() const {
            return _exprs;
        }

        void evaluate(const ExpressionContext& context) const {
            for (auto it = _bindingMap.begin(); it != _bindingMap.end(); it++) {
                const BindingVariant& binding = it->second;
                boost::apply_visitor(exprbinderimpl::Setter(context), binding);
            }
        }

    private:
        template <typename V>
        struct Binding {
            V* field;
            std::shared_ptr<Expression> expr;
            const T* object = nullptr;
            V(*convertFn)(const Value&) = nullptr;
            V (T::*memberConvertFn)(const Value&) const = nullptr;

            explicit Binding(V* field, std::shared_ptr<Expression> expr, V(*convertFn)(const Value&)) : field(field), expr(std::move(expr)), convertFn(convertFn) { }
            explicit Binding(V* field, std::shared_ptr<Expression> expr, V (T::*memberConvertFn)(const Value&) const, const T* object) : field(field), expr(std::move(expr)), memberConvertFn(memberConvertFn), object(object) { }
        };

        using BindingVariant = boost::variant<Binding<bool>, Binding<int>,  Binding<float>, Binding<std::string>, Binding<vt::Color>, Binding<cglib::mat3x3<float>>, Binding<boost::optional<cglib::mat3x3<float>>>>;

        std::vector<std::shared_ptr<Expression>> _exprs;
        std::map<void *, BindingVariant> _bindingMap;
    };
} }

#endif
