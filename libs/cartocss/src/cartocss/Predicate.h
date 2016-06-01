/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSS_PREDICATE_H_
#define _CARTO_CARTOCSS_PREDICATE_H_

#include "Value.h"
#include "Expression.h"

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <functional>

#include <boost/variant.hpp>
#include <boost/logic/tribool.hpp>

namespace carto { namespace css {
    class PredicateContext {
    public:
        std::string layerName;
        ExpressionContext expressionContext;

        PredicateContext() = default;
    };

    class Predicate : public std::enable_shared_from_this<Predicate> {
    public:
        virtual ~Predicate() = default;

        virtual boost::tribool evaluate(const PredicateContext& context) const = 0;

        virtual boost::tribool contains(const std::shared_ptr<const Predicate>& pred) const = 0;
        virtual boost::tribool intersects(const std::shared_ptr<const Predicate>& pred) const = 0;
    };

    class MapPredicate : public Predicate {
    public:
        MapPredicate() = default;

        virtual boost::tribool evaluate(const PredicateContext& context) const override {
            return context.layerName.empty();
        }

        virtual boost::tribool contains(const std::shared_ptr<const Predicate>& pred) const override {
            return boost::indeterminate;
        }

        virtual boost::tribool intersects(const std::shared_ptr<const Predicate>& pred) const override {
            return contains(pred);
        }
    };

    class LayerPredicate : public Predicate {
    public:
        explicit LayerPredicate(std::string layerName) : _layerName(std::move(layerName)) { }

        const std::string& getLayerName() const { return _layerName; }

        virtual boost::tribool evaluate(const PredicateContext& context) const override {
            return context.layerName == _layerName;
        }

        virtual boost::tribool contains(const std::shared_ptr<const Predicate>& pred) const override {
            if (auto layerPred = std::dynamic_pointer_cast<const LayerPredicate>(pred)) {
                return _layerName == layerPred->_layerName;
            }
            return boost::indeterminate;
        }

        virtual boost::tribool intersects(const std::shared_ptr<const Predicate>& pred) const override {
            return contains(pred);
        }

    private:
        std::string _layerName;
    };

    class ClassPredicate : public Predicate {
    public:
        explicit ClassPredicate(std::string cls) : _cls(std::move(cls)) { }

        const std::string& getClass() const { return _cls; }

        virtual boost::tribool evaluate(const PredicateContext& context) const override {
            if (!context.expressionContext.fieldMap) {
                return boost::indeterminate;
            }
            auto it = context.expressionContext.fieldMap->find("class");
            if (it == context.expressionContext.fieldMap->end()) {
                return boost::indeterminate;
            }
            if (auto cls = boost::get<std::string>(&it->second)) {
                return *cls == _cls;
            }
            return false;
        }

        virtual boost::tribool contains(const std::shared_ptr<const Predicate>& pred) const override {
            if (auto clsPred = std::dynamic_pointer_cast<const ClassPredicate>(pred)) {
                return _cls == clsPred->_cls;
            }
            return boost::indeterminate;
        }

        virtual boost::tribool intersects(const std::shared_ptr<const Predicate>& pred) const override {
            return contains(pred);
        }

    private:
        std::string _cls;
    };

    class OpPredicate : public Predicate {
    public:
        enum class Op {
            EQ,
            NEQ,
            LT,
            LTE,
            GT,
            GTE,
            MATCH
        };

        explicit OpPredicate(Op op, bool field, std::string fieldOrVar, Value refValue) : _op(op), _field(field), _fieldOrVar(std::move(fieldOrVar)), _refValue(std::move(refValue)) { }

        Op getOp() const { return _op; }
        bool isField() const { return _field; }
        const std::string& getFieldOrVar() const { return _fieldOrVar; }
        const Value& getRefValue() const { return _refValue; }

        virtual boost::tribool evaluate(const PredicateContext& context) const override {
            Value fieldOrVarValue;
            bool fieldOrVarExists = false;
            if (!_field) {
                if (context.expressionContext.variableMap) {
                    auto it = context.expressionContext.variableMap->find(_fieldOrVar);
                    if (it != context.expressionContext.variableMap->end()) {
                        boost::variant<Value, std::shared_ptr<const Expression>> result = it->second->evaluate(context.expressionContext);
                        if (auto val = boost::get<Value>(&result)) {
                            fieldOrVarValue = *val;
                            fieldOrVarExists = true;
                        }
                    }
                }
                if (context.expressionContext.predefinedVariableMap) {
                    auto it = context.expressionContext.predefinedVariableMap->find(_fieldOrVar);
                    if (it != context.expressionContext.predefinedVariableMap->end()) {
                        fieldOrVarValue = it->second;
                        fieldOrVarExists = true;
                    }
                }
            }
            else {
                if (context.expressionContext.fieldMap) {
                    auto it = context.expressionContext.fieldMap->find(_fieldOrVar);
                    if (it != context.expressionContext.fieldMap->end()) {
                        fieldOrVarValue = it->second;
                        fieldOrVarExists = true;
                    }
                }
                if (context.expressionContext.predefinedFieldMap) {
                    auto it = context.expressionContext.predefinedFieldMap->find(_fieldOrVar);
                    if (it != context.expressionContext.predefinedFieldMap->end()) {
                        fieldOrVarValue = it->second;
                        fieldOrVarExists = true;
                    }
                }
            }

            if (fieldOrVarExists) {
                return applyOp(_op, fieldOrVarValue, _refValue);
            }
            return boost::indeterminate;
        }

        virtual boost::tribool contains(const std::shared_ptr<const Predicate>& pred) const override {
            if (auto opPred = std::dynamic_pointer_cast<const OpPredicate>(pred)) {
                if (_field == opPred->_field && _fieldOrVar == opPred->_fieldOrVar) {
                    const Value& val1 = _refValue;
                    const Value& val2 = opPred->_refValue;
                    if (opPred->_op == Op::EQ) {
                        return applyOp(_op, val2, val1);
                    }
                    if (_op == Op::NEQ) {
                        return !applyOp(opPred->_op, val1, val2);
                    }
                    if (_op == Op::LT && (opPred->_op == Op::LT || opPred->_op == Op::LTE)) {
                        return !applyOp(opPred->_op, val1, val2);
                    }
                    if (_op == Op::LTE && (opPred->_op == Op::LT || opPred->_op == Op::LTE)) {
                        return applyOp(_op, val2, val1);
                    }
                    if (_op == Op::GT && (opPred->_op == Op::GT || opPred->_op == Op::GTE)) {
                        return !applyOp(opPred->_op, val1, val2);
                    }
                    if (_op == Op::GTE && (opPred->_op == Op::GT || opPred->_op == Op::GTE)) {
                        return applyOp(_op, val2, val1);
                    }
                    if (_op == Op::MATCH) {
                        return boost::indeterminate;
                    }
                    return false;
                }
            }
            return boost::indeterminate;
        }

        virtual boost::tribool intersects(const std::shared_ptr<const Predicate>& pred) const override {
            if (auto opPred = std::dynamic_pointer_cast<const OpPredicate>(pred)) {
                if (_field == opPred->_field && _fieldOrVar == opPred->_fieldOrVar) {
                    const Value& val1 = _refValue;
                    const Value& val2 = opPred->_refValue;
                    if (_op == Op::EQ) {
                        return applyOp(opPred->_op, val1, val2);
                    }
                    if (opPred->_op == Op::EQ) {
                        return applyOp(_op, val2, val1);
                    }
                    if (_op == Op::LTE && (opPred->_op == Op::GT || opPred->_op == Op::GTE)) {
                        return applyOp(opPred->_op, val1, val2);
                    }
                    if (_op == Op::LT && (opPred->_op == Op::GT || opPred->_op == Op::GTE)) {
                        return applyOp(_op, val2, val1);
                    }
                    if (_op == Op::GTE && (opPred->_op == Op::LT || opPred->_op == Op::LTE)) {
                        return applyOp(opPred->_op, val1, val2);
                    }
                    if (_op == Op::GT && (opPred->_op == Op::LT || opPred->_op == Op::LTE)) {
                        return applyOp(_op, val2, val1);
                    }
                    if (_op == Op::MATCH) {
                        return boost::indeterminate;
                    }
                    return true;
                }
            }
            return boost::indeterminate;
        }

    private:
        template <template <typename> class Op>
        struct CompareOp : boost::static_visitor<boost::tribool> {
            boost::tribool operator()(boost::blank val1, boost::blank val2) { return Op<boost::blank>()(val1, val2); }
            boost::tribool operator()(long long val1, long long val2) const { return Op<long long>()(val1, val2); }
            boost::tribool operator()(long long val1, double val2) const { return Op<double>()(static_cast<double>(val1), val2); }
            boost::tribool operator()(double val1, long long val2) const { return Op<double>()(val1, static_cast<double>(val2)); }
            boost::tribool operator()(double val1, double val2) const { return Op<double>()(val1, val2); }
            boost::tribool operator()(Color val1, Color val2) const { return Op<Color>()(val1, val2); }
            boost::tribool operator()(const std::string& val1, const std::string& val2) const { return Op<std::string>()(val1, val2); }
            template <typename S, typename T> boost::tribool operator()(S val1, T val2) const { return boost::indeterminate; }
        };

        static boost::tribool applyOp(Op op, const Value& val1, const Value& val2) {
            switch (op) {
            case Op::EQ:
                if (boost::apply_visitor(CompareOp<std::equal_to>(), val1, val2)) {
                    return true;
                }
                return false;
            case Op::NEQ:
                if (!boost::apply_visitor(CompareOp<std::not_equal_to>(), val1, val2)) {
                    return false;
                }
                return true;
            case Op::LT:
                return boost::apply_visitor(CompareOp<std::less>(), val1, val2);
            case Op::LTE:
                return boost::apply_visitor(CompareOp<std::less_equal>(), val1, val2);
            case Op::GT:
                return boost::apply_visitor(CompareOp<std::greater>(), val1, val2);
            case Op::GTE:
                return boost::apply_visitor(CompareOp<std::greater_equal>(), val1, val2);
            case Op::MATCH:
                return boost::indeterminate;
            }
            return boost::indeterminate;
        }

        Op _op;
        bool _field;
        std::string _fieldOrVar;
        Value _refValue;
    };

    class AttachmentPredicate : public Predicate {
    public:
        explicit AttachmentPredicate(std::string attachment) : _attachment(std::move(attachment)) { }

        const std::string& getAttachment() const { return _attachment; }

        virtual boost::tribool evaluate(const PredicateContext& context) const override {
            return boost::indeterminate;
        }

        virtual boost::tribool contains(const std::shared_ptr<const Predicate>& pred) const override {
            return boost::indeterminate;
        }

        virtual boost::tribool intersects(const std::shared_ptr<const Predicate>& pred) const override {
            return contains(pred);
        }

    private:
        std::string _attachment;
    };
} }

#endif
