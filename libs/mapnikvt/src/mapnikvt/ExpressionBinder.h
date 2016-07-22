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
#include "vt/Styles.h"

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
        ExpressionBinder& bind(V* field, const std::shared_ptr<const Expression>& expr) {
            if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                *field = ValueConverter<V>::convert(constExpr->getConstant());
            }
            else {
                _bindingMap.insert({ field, Binding<V>(field, expr, &ValueConverter<V>::convert) });
            }
            return *this;
        }

        template <typename V>
        ExpressionBinder& bind(V* field, const std::shared_ptr<const Expression>& expr, V(*convertFn)(const Value& val)) {
            if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                *field = (*convertFn)(constExpr->getConstant());
            }
            else {
                _bindingMap.insert({ field, Binding<V>(field, expr, convertFn) });
            }
            return *this;
        }

        template <typename V>
        ExpressionBinder& bind(V* field, const std::shared_ptr<const Expression>& expr, V (T::*memberConvertFn)(const Value& val) const, const T* object) {
            if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                *field = (object->*memberConvertFn)(constExpr->getConstant());
            }
            else {
                _bindingMap.insert({ field, Binding<V>(field, expr, memberConvertFn, object) });
            }
            return *this;
        }

        void evaluate(const FeatureExpressionContext& context) const {
            for (auto it = _bindingMap.begin(); it != _bindingMap.end(); it++) {
                const BindingVariant& binding = it->second;
                boost::apply_visitor(exprbinderimpl::Setter(context), binding);
            }
        }

    private:
        template <typename V>
        struct Binding {
            V* field;
            std::shared_ptr<const Expression> expr;
            V(*convertFn)(const Value&) = nullptr;
            V (T::*memberConvertFn)(const Value&) const = nullptr;
            const T* object = nullptr;

            explicit Binding(V* field, std::shared_ptr<const Expression> expr, V(*convertFn)(const Value&)) : field(field), expr(std::move(expr)), convertFn(convertFn) { }
            explicit Binding(V* field, std::shared_ptr<const Expression> expr, V (T::*memberConvertFn)(const Value&) const, const T* object) : field(field), expr(std::move(expr)), memberConvertFn(memberConvertFn), object(object) { }
        };

        using BindingVariant = boost::variant<Binding<bool>, Binding<int>, Binding<float>, Binding<std::string>, Binding<vt::Color>, Binding<cglib::mat3x3<float>>, Binding<boost::optional<cglib::mat3x3<float>>>>;

        std::vector<std::shared_ptr<const Expression>> _exprs;
        std::map<void *, BindingVariant> _bindingMap;
    };

    class ExpressionFunctionBinder {
    public:
        ExpressionFunctionBinder() = default;

        ExpressionFunctionBinder& bind(std::shared_ptr<const vt::FloatFunction>* field, const std::shared_ptr<const Expression>& expr) {
            if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                *field = buildFloatFunction(expr);
            }
            else {
                _bindingMap.insert({ field, expr });
            }
            return *this;
        }

        void evaluate(const FeatureExpressionContext& context) const {
            for (auto it = _bindingMap.begin(); it != _bindingMap.end(); it++) {
                std::shared_ptr<const Expression> expr = simplifyExpression(it->second, context);
                *it->first = buildFloatFunction(expr);
            }
        }

    private:
        std::shared_ptr<const vt::FloatFunction> buildFloatFunction(const std::shared_ptr<const Expression>& expr) const {
            for (auto it = _functionCache.begin(); it != _functionCache.end(); it++) {
                if (it->first->equals(expr)) {
                    return it->second;
                }
            }
            auto func = std::make_shared<vt::FloatFunction>([expr](const vt::ViewState& viewState) {
                ViewExpressionContext context;
                context.setZoom(viewState.zoom);
                return ValueConverter<float>::convert(expr->evaluate(context));
            });
            if (_functionCache.size() > 16) {
                _functionCache.erase(_functionCache.begin()); // erase any element to keep cache compact
            }
            _functionCache[expr] = func;
            return func;
        }

        static std::shared_ptr<const Expression> simplifyExpression(const std::shared_ptr<const Expression>& expr, const FeatureExpressionContext& context) {
            return expr->map([&context](const std::shared_ptr<const Expression>& expr) -> std::shared_ptr<const Expression> {
                bool containsViewVariables = false;
                expr->fold([&containsViewVariables, &context](const std::shared_ptr<const Expression>& expr) {
                    if (auto varExpr = std::dynamic_pointer_cast<const VariableExpression>(expr)) {
                        std::string name = varExpr->getVariableName(context);
                        if (ViewExpressionContext::isViewVariable(name)) {
                            containsViewVariables = true;
                        }
                    }
                });
                if (containsViewVariables) {
                    return expr;
                }
                Value val = expr->evaluate(context);
                return std::make_shared<ConstExpression>(val);
            });
        }

        std::map<std::shared_ptr<const vt::FloatFunction>*, std::shared_ptr<const Expression>> _bindingMap;
        mutable std::map<std::shared_ptr<const Expression>, std::shared_ptr<const vt::FloatFunction>> _functionCache;
    };
} }

#endif
