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

#include <memory>
#include <functional>

namespace carto { namespace mvt {
    template <typename V>
    class ExpressionBinder final {
    public:
        ExpressionBinder() = default;

        ExpressionBinder& bind(V* field, const std::shared_ptr<const Expression>& expr) {
            return bind(field, expr, std::function<V(const Value&)>(ValueConverter<V>::convert));
        }
        
        ExpressionBinder& bind(V* field, const std::shared_ptr<const Expression>& expr, std::function<V(const Value& val)> convertFunc) {
            if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                *field = convertFunc(constExpr->getConstant());
            }
            else {
                _bindingMap.insert({ field, Binding(expr, std::move(convertFunc)) });
            }
            return *this;
        }

        void update(const FeatureExpressionContext& context) const {
            for (auto it = _bindingMap.begin(); it != _bindingMap.end(); it++) {
                const Binding& binding = it->second;
                Value val = binding.expr->evaluate(context);
                *it->first = binding.convertFunc(val);
            }
        }

    private:
        struct Binding {
            std::shared_ptr<const Expression> expr;
            std::function<V(const Value&)> convertFunc;

            explicit Binding(std::shared_ptr<const Expression> expr, std::function<V(const Value&)> convertFunc) : expr(std::move(expr)), convertFunc(std::move(convertFunc)) { }
        };

        std::map<V*, Binding> _bindingMap;
    };

    template <typename V>
    class ExpressionFunctionBinder final {
    public:
        using FuncPtr = std::shared_ptr<const std::function<V(const vt::ViewState&)>>;

        ExpressionFunctionBinder() = default;

        ExpressionFunctionBinder& bind(FuncPtr* field, const std::shared_ptr<const Expression>& expr) {
            return bind(field, expr, std::function<V(const Value&)>(ValueConverter<V>::convert));
        }

        ExpressionFunctionBinder& bind(FuncPtr* field, const std::shared_ptr<const Expression>& expr, std::function<V(const Value&)> convertFunc) {
            if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                *field = buildFunction(expr, convertFunc);
            }
            else {
                _bindingMap.insert({ field, Binding(expr, convertFunc) });
            }
            return *this;
        }

        void update(const FeatureExpressionContext& context) const {
            for (auto it = _bindingMap.begin(); it != _bindingMap.end(); it++) {
                const Binding& binding = it->second;
                std::shared_ptr<const Expression> expr = simplifyExpression(binding.expr, context);
                *it->first = buildFunction(expr, binding.convertFunc);
            }
        }

    private:
        struct Binding {
            std::shared_ptr<const Expression> expr;
            std::function<V(const Value&)> convertFunc;

            explicit Binding(std::shared_ptr<const Expression> expr, std::function<V(const Value&)> convertFunc) : expr(std::move(expr)), convertFunc(std::move(convertFunc)) { }
        };

        FuncPtr buildFunction(const std::shared_ptr<const Expression>& expr, const std::function<V(const Value&)>& convertFunc) const {
            for (auto it = _functionCache.begin(); it != _functionCache.end(); it++) {
                if (it->first->equals(expr)) {
                    return it->second;
                }
            }
            FuncPtr func;
            if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                V val = convertFunc(constExpr->getConstant());
                func = std::make_shared<const std::function<V(const vt::ViewState&)>>([val](const vt::ViewState& viewState) {
                    return val;
                });
            }
            else {
                func = std::make_shared<const std::function<V(const vt::ViewState&)>>([expr, convertFunc](const vt::ViewState& viewState) {
                    ViewExpressionContext context;
                    context.setZoom(viewState.zoom);
                    return convertFunc(expr->evaluate(context));
                });
            }
            if (_functionCache.size() >= 32) { // 32 is a good fit if function depends on 'discrete zoom'
                _functionCache.erase(_functionCache.begin()); // erase any element to keep the cache compact
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

        std::map<FuncPtr*, Binding> _bindingMap;
        mutable std::map<std::shared_ptr<const Expression>, FuncPtr> _functionCache;
    };
} }

#endif
