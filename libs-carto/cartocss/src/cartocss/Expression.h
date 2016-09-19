/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSS_EXPRESSION_H_
#define _CARTO_CARTOCSS_EXPRESSION_H_

#include "Value.h"
#include "mapnikvt/StringUtils.h"

#include <memory>
#include <string>
#include <array>
#include <vector>
#include <map>
#include <utility>
#include <functional>

#include <boost/lexical_cast.hpp>

namespace carto { namespace css {
    class Expression;

    class ExpressionContext {
    public:
        std::map<std::string, Value>* predefinedVariableMap = nullptr;
        std::map<std::string, std::shared_ptr<const Expression>>* variableMap = nullptr;

        std::map<std::string, Value>* predefinedFieldMap = nullptr;
        std::map<std::string, Value>* fieldMap = nullptr;

        ExpressionContext() = default;
    };

    class Expression : public std::enable_shared_from_this<Expression> {
    public:
        virtual ~Expression() = default;

        virtual boost::variant<Value, std::shared_ptr<const Expression>> evaluate(const ExpressionContext& context) const = 0;

        virtual bool equals(const Expression& expr) const = 0;
    };

    class ConstExpression : public Expression {
    public:
        explicit ConstExpression(Value value) : _value(std::move(value)) { }

        const Value& getValue() const { return _value; }

        virtual boost::variant<Value, std::shared_ptr<const Expression>> evaluate(const ExpressionContext& context) const override {
            return _value;
        }

        virtual bool equals(const Expression& expr) const override {
            if (auto constExpr = dynamic_cast<const ConstExpression*>(&expr)) {
                return _value == constExpr->_value;
            }
            return false;
        }

    private:
        Value _value;
    };

    class FieldOrVarExpression : public Expression {
    public:
        explicit FieldOrVarExpression(bool field, std::string fieldOrVar) : _field(field), _fieldOrVar(std::move(fieldOrVar)) { }

        bool isField() const { return _field; }
        const std::string& getFieldOrVar() const { return _fieldOrVar; }

        virtual boost::variant<Value, std::shared_ptr<const Expression>> evaluate(const ExpressionContext& context) const override {
            if (!_field) {
                if (context.predefinedVariableMap) {
                    auto it = context.predefinedVariableMap->find(_fieldOrVar);
                    if (it != context.predefinedVariableMap->end()) {
                        return it->second;
                    }
                }
                if (context.variableMap) {
                    auto it = context.variableMap->find(_fieldOrVar);
                    if (it != context.variableMap->end()) {
                        return it->second->evaluate(context);
                    }
                }
            }
            else {
                if (context.predefinedFieldMap) {
                    auto it = context.predefinedFieldMap->find(_fieldOrVar);
                    if (it != context.predefinedFieldMap->end()) {
                        return it->second;
                    }
                }
                if (context.fieldMap) {
                    auto it = context.fieldMap->find(_fieldOrVar);
                    if (it != context.fieldMap->end()) {
                        return it->second;
                    }
                }
            }
            return std::static_pointer_cast<const Expression>(shared_from_this());
        }

        virtual bool equals(const Expression& expr) const override {
            if (auto fieldOrVarExpr = dynamic_cast<const FieldOrVarExpression*>(&expr)) {
                return _field == fieldOrVarExpr->_field && _fieldOrVar == fieldOrVarExpr->_fieldOrVar;
            }
            return false;
        }

    private:
        bool _field;
        std::string _fieldOrVar;
    };

    class ListExpression : public Expression {
    public:
        explicit ListExpression(std::vector<std::shared_ptr<const Expression>> exprs) : _exprs(std::move(exprs)) { }

        const std::vector<std::shared_ptr<const Expression>>& getExpressions() const { return _exprs; }

        virtual boost::variant<Value, std::shared_ptr<const Expression>> evaluate(const ExpressionContext& context) const override {
            std::vector<Value> vals;
            vals.reserve(_exprs.size());
            for (const std::shared_ptr<const Expression>& expr : _exprs) {
                boost::variant<Value, std::shared_ptr<const Expression>> result = expr->evaluate(context);
                if (auto val = boost::get<Value>(&result)) {
                    vals.push_back(*val);
                }
                else {
                    break;
                }
            }
            if (vals.size() == _exprs.size()) {
                return Value(std::move(vals));
            }
            std::vector<std::shared_ptr<const Expression>> exprs;
            exprs.reserve(_exprs.size());
            for (const std::shared_ptr<const Expression>& expr : _exprs) {
                boost::variant<Value, std::shared_ptr<const Expression>> result = expr->evaluate(context);
                if (auto val = boost::get<Value>(&result)) {
                    exprs.push_back(std::make_shared<ConstExpression>(*val));
                }
                else {
                    exprs.push_back(boost::get<std::shared_ptr<const Expression>>(result));
                }
            }
            return std::static_pointer_cast<const Expression>(std::make_shared<ListExpression>(std::move(exprs)));
        }

        virtual bool equals(const Expression& expr) const override {
            if (auto listExpr = dynamic_cast<const ListExpression*>(&expr)) {
                if (_exprs.size() == listExpr->_exprs.size()) {
                    return std::equal(_exprs.begin(), _exprs.end(), listExpr->_exprs.begin(), [](const std::shared_ptr<const Expression>& expr1, const std::shared_ptr<const Expression>& expr2) {
                        return expr1->equals(*expr2);
                    });
                }
            }
            return false;
        }

    private:
        std::vector<std::shared_ptr<const Expression>> _exprs;
    };

    class UnaryExpression : public Expression {
    public:
        enum class Op {
            NOT,
            NEG
        };

        explicit UnaryExpression(Op op, std::shared_ptr<const Expression> expr) : _op(op), _expr(std::move(expr)) { }

        Op getOp() const { return _op; }
        const std::shared_ptr<const Expression>& getExpression() const { return _expr; }

        virtual boost::variant<Value, std::shared_ptr<const Expression>> evaluate(const ExpressionContext& context) const override {
            boost::variant<Value, std::shared_ptr<const Expression>> result = _expr->evaluate(context);
            if (auto val = boost::get<Value>(&result)) {
                return applyOp(_op, *val);
            }
            std::shared_ptr<const Expression> expr = boost::get<std::shared_ptr<const Expression>>(result);
            return std::static_pointer_cast<const Expression>(std::make_shared<UnaryExpression>(_op, std::move(expr)));
        }

        virtual bool equals(const Expression& expr) const override {
            if (auto unaryExpr = dynamic_cast<const UnaryExpression*>(&expr)) {
                return _op == unaryExpr->_op && _expr->equals(*unaryExpr->_expr);
            }
            return false;
        }

    private:
        struct NotOp : boost::static_visitor<Value> {
            Value operator() (bool val) const { return Value(!val); }
            template <typename T> Value operator() (T val) const { throw std::runtime_error("Unexpected type in ! operator"); }
        };
        
        template <template <typename> class OpImpl>
        struct ArithmeticOp : boost::static_visitor<Value> {
            Value operator() (long long val) const { return Value(OpImpl<long long>()(val)); }
            Value operator() (double val) const { return Value(OpImpl<double>()(val)); }
            template <typename T> Value operator() (T val) const { throw std::runtime_error("Unexpected type in unary operator"); }
        };

        static Value applyOp(Op op, const Value& val) {
            switch (op) {
            case Op::NOT:
                return boost::apply_visitor(NotOp(), val);
            case Op::NEG:
                return boost::apply_visitor(ArithmeticOp<std::negate>(), val);
            }
            throw std::runtime_error("Unsupported unary operation");
        }

        Op _op;
        std::shared_ptr<const Expression> _expr;
    };

    class BinaryExpression : public Expression {
    public:
        enum class Op {
            AND,
            OR,
            EQ,
            NEQ,
            LT,
            LTE,
            GT,
            GTE,
            MATCH,
            ADD,
            SUB,
            MUL,
            DIV
        };

        explicit BinaryExpression(Op op, std::shared_ptr<const Expression> expr1, std::shared_ptr<const Expression> expr2) : _op(op), _expr1(std::move(expr1)), _expr2(std::move(expr2)) { }

        Op getOp() const { return _op; }
        const std::shared_ptr<const Expression>& getExpression1() const { return _expr1; }
        const std::shared_ptr<const Expression>& getExpression2() const { return _expr2; }

        virtual boost::variant<Value, std::shared_ptr<const Expression>> evaluate(const ExpressionContext& context) const override {
            boost::variant<Value, std::shared_ptr<const Expression>> result1 = _expr1->evaluate(context);
            boost::variant<Value, std::shared_ptr<const Expression>> result2 = _expr2->evaluate(context);
            if (auto val1 = boost::get<Value>(&result1)) {
                if (auto val2 = boost::get<Value>(&result2)) {
                    return applyOp(_op, *val1, *val2);
                }
            }
            std::shared_ptr<const Expression> expr1;
            if (auto val1 = boost::get<Value>(&result1)) {
                expr1 = std::make_shared<ConstExpression>(*val1);
            }
            else {
                expr1 = boost::get<std::shared_ptr<const Expression>>(result1);
            }
            std::shared_ptr<const Expression> expr2;
            if (auto val2 = boost::get<Value>(&result2)) {
                expr2 = std::make_shared<ConstExpression>(*val2);
            }
            else {
                expr2 = boost::get<std::shared_ptr<const Expression>>(result2);
            }
            return std::static_pointer_cast<const Expression>(std::make_shared<BinaryExpression>(_op, std::move(expr1), std::move(expr2)));
        }

        virtual bool equals(const Expression& expr) const override {
            if (auto binaryExpr = dynamic_cast<const BinaryExpression*>(&expr)) {
                return _op == binaryExpr->_op && _expr1->equals(*binaryExpr->_expr1) && _expr2->equals(*binaryExpr->_expr2);
            }
            return false;
        }

    private:
        struct AndOp : boost::static_visitor<Value> {
            Value operator() (bool val1, bool val2) const { return Value(val1 && val2); }
            template <typename S, typename T> Value operator() (S val1, T val2) const { throw std::runtime_error("Unexpected types in binary && operator"); }
        };

        struct OrOp : boost::static_visitor<Value> {
            Value operator() (bool val1, bool val2) const { return Value(val1 || val2); }
            template <typename S, typename T> Value operator() (S val1, T val2) const { throw std::runtime_error("Unexpected types in binary || operator"); }
        };

        template <template <typename> class OpImpl, bool NullResult, bool MismatchResult>
        struct CompOp : boost::static_visitor<Value> {
            Value operator() (boost::blank, boost::blank) const { return Value(NullResult); }
            Value operator() (bool val1, long long val2) const { return Value(OpImpl<long long>()(static_cast<long long>(val1), val2)); }
            Value operator() (bool val1, double val2) const { return Value(OpImpl<double>()(static_cast<double>(val1), val2)); }
            Value operator() (long long val1, bool val2) const { return Value(OpImpl<long long>()(val1, static_cast<long long>(val2))); }
            Value operator() (long long val1, double val2) const { return Value(OpImpl<double>()(static_cast<double>(val1), val2)); }
            Value operator() (double val1, bool val2) const { return Value(OpImpl<double>()(val1, static_cast<double>(val2))); }
            Value operator() (double val1, long long val2) const { return Value(OpImpl<double>()(val1, static_cast<double>(val2))); }
            template <typename T> Value operator() (T val1, T val2) const { return Value(OpImpl<T>()(val1, val2)); }
            template <typename S, typename T> Value operator() (S val1, T val2) const { return Value(MismatchResult); }
        };

        struct MatchOp : boost::static_visitor<Value> {
            Value operator() (const std::string& val1, const std::string& val2) const { return Value(mvt::regexMatch(val1, val2)); }
            template <typename S, typename T> Value operator() (S val1, T val2) const { return Value(false); }
        };

        struct AddOp : boost::static_visitor<Value> {
            Value operator() (long long val1, long long val2) const { return Value(val1 + val2); }
            Value operator() (long long val1, double val2) const { return Value(static_cast<double>(val1) + val2); }
            Value operator() (double val1, long long val2) const { return Value(val1 + static_cast<double>(val2)); }
            Value operator() (double val1, double val2) const { return Value(val1 + val2); }
            Value operator() (Color val1, Color val2) const { return Value(val1 + val2); }
            Value operator() (const std::string& val1, const std::string& val2) const { return Value(val1 + val2); }
            template <typename S, typename T> Value operator() (S val1, T val2) const { throw std::runtime_error("Unexpected types in binary + operator"); }
        };

        struct SubOp : boost::static_visitor<Value> {
            Value operator() (long long val1, long long val2) const { return Value(val1 - val2); }
            Value operator() (long long val1, double val2) const { return Value(static_cast<double>(val1) - val2); }
            Value operator() (double val1, long long val2) const { return Value(val1 - static_cast<double>(val2)); }
            Value operator() (double val1, double val2) const { return Value(val1 - val2); }
            Value operator() (Color val1, Color val2) const { return Value(val1 - val2); }
            template <typename S, typename T> Value operator() (S val1, T val2) const { throw std::runtime_error("Unexpected types in binary - operator"); }
        };

        struct MulOp : boost::static_visitor<Value> {
            Value operator() (long long val1, long long val2) const { return Value(val1 * val2); }
            Value operator() (long long val1, double val2) const { return Value(static_cast<double>(val1) * val2); }
            Value operator() (double val1, long long val2) const { return Value(val1 * static_cast<double>(val2)); }
            Value operator() (double val1, double val2) const { return Value(val1 * val2); }
            Value operator() (Color val1, Color val2) const { return Value(val1 * val2); }
            Value operator() (Color val1, long long val2) const { return Value(val1 * static_cast<float>(val2)); }
            Value operator() (Color val1, double val2) const { return Value(val1 * static_cast<float>(val2)); }
            Value operator() (long long val1, Color val2) const { return Value(static_cast<float>(val1) * val2); }
            Value operator() (double val1, Color val2) const { return Value(static_cast<float>(val1) * val2); }
            template <typename S, typename T> Value operator() (S val1, T val2) const { throw std::runtime_error("Unexpected types in binary * operator"); }
        };

        struct DivOp : boost::static_visitor<Value> {
            Value operator() (long long val1, long long val2) const { if (val2 == 0) { throw std::runtime_error("Division with 0"); } return Value(val1 / val2); }
            Value operator() (long long val1, double val2) const { return Value(static_cast<double>(val1) / val2); }
            Value operator() (double val1, long long val2) const { return Value(val1 / static_cast<double>(val2)); }
            Value operator() (double val1, double val2) const { return Value(val1 / val2); }
            Value operator() (Color val1, Color val2) const { return Value(val1 / val2); }
            Value operator() (Color val1, long long val2) const { return Value(val1 * (1.0f / static_cast<float>(val2))); }
            Value operator() (Color val1, double val2) const { return Value(val1 * (1.0f / static_cast<float>(val2))); }
            template <typename S, typename T> Value operator() (S val1, T val2) const { throw std::runtime_error("Unexpected types in binary / operator"); }
        };

        static Value applyOp(Op op, const Value& val1, const Value& val2) {
            switch (op) {
            case Op::AND:
                return boost::apply_visitor(AndOp(), val1, val2);
            case Op::OR:
                return boost::apply_visitor(OrOp(), val1, val2);
            case Op::EQ:
                return boost::apply_visitor(CompOp<std::equal_to, true, false>(), val1, val2);
            case Op::NEQ:
                return boost::apply_visitor(CompOp<std::not_equal_to, false, true>(), val1, val2);
            case Op::LT:
                return boost::apply_visitor(CompOp<std::less, false, false>(), val1, val2);
            case Op::LTE:
                return boost::apply_visitor(CompOp<std::less_equal, true, false>(), val1, val2);
            case Op::GT:
                return boost::apply_visitor(CompOp<std::greater, false, false>(), val1, val2);
            case Op::GTE:
                return boost::apply_visitor(CompOp<std::greater_equal, true, false>(), val1, val2);
            case Op::MATCH:
                return boost::apply_visitor(MatchOp(), val1, val2);
            case Op::ADD:
                if (auto str1 = boost::get<std::string>(&val1)) {
                    return *str1 + boost::lexical_cast<std::string>(val2);
                }
                else if (auto str2 = boost::get<std::string>(&val2)) {
                    return boost::lexical_cast<std::string>(val1) + *str2;
                }
                return boost::apply_visitor(AddOp(), val1, val2);
            case Op::SUB:
                return boost::apply_visitor(SubOp(), val1, val2);
            case Op::MUL:
                return boost::apply_visitor(MulOp(), val1, val2);
            case Op::DIV:
                return boost::apply_visitor(DivOp(), val1, val2);
            }
            throw std::runtime_error("Unsupported binary operation");
        }

        Op _op;
        std::shared_ptr<const Expression> _expr1;
        std::shared_ptr<const Expression> _expr2;
    };

    class ConditionalExpression : public Expression {
    public:
        explicit ConditionalExpression(std::shared_ptr<const Expression> cond, std::shared_ptr<const Expression> expr1, std::shared_ptr<const Expression> expr2) : _cond(std::move(cond)), _expr1(std::move(expr1)), _expr2(std::move(expr2)) { }

        const std::shared_ptr<const Expression>& getCondition() const { return _cond; }
        const std::shared_ptr<const Expression>& getExpression1() const { return _expr1; }
        const std::shared_ptr<const Expression>& getExpression2() const { return _expr2; }

        virtual boost::variant<Value, std::shared_ptr<const Expression>> evaluate(const ExpressionContext& context) const override {
            boost::variant<Value, std::shared_ptr<const Expression>> condResult = _cond->evaluate(context);
            boost::variant<Value, std::shared_ptr<const Expression>> result1 = _expr1->evaluate(context);
            boost::variant<Value, std::shared_ptr<const Expression>> result2 = _expr2->evaluate(context);

            if (auto condVal = boost::get<Value>(&condResult)) {
                if (auto cond = boost::get<bool>(condVal)) {
                    return *cond ? result1 : result2;
                }
                else {
                    throw std::runtime_error("Condition type error, expecting boolean");
                }
            }

            std::shared_ptr<const Expression> expr1;
            if (auto val1 = boost::get<Value>(&result1)) {
                expr1 = std::make_shared<ConstExpression>(*val1);
            }
            else {
                expr1 = boost::get<std::shared_ptr<const Expression>>(result1);
            }
            std::shared_ptr<const Expression> expr2;
            if (auto val2 = boost::get<Value>(&result2)) {
                expr2 = std::make_shared<ConstExpression>(*val2);
            }
            else {
                expr2 = boost::get<std::shared_ptr<const Expression>>(result2);
            }
            return std::static_pointer_cast<const Expression>(std::make_shared<ConditionalExpression>(boost::get<std::shared_ptr<const Expression>>(condResult), std::move(expr1), std::move(expr2)));
        }

        virtual bool equals(const Expression& expr) const override {
            if (auto condExpr = dynamic_cast<const ConditionalExpression*>(&expr)) {
                return _cond->equals(*condExpr->_cond) && _expr1->equals(*condExpr->_expr1) && _expr2->equals(*condExpr->_expr2);
            }
            return false;
        }

    private:
        std::shared_ptr<const Expression> _cond;
        std::shared_ptr<const Expression> _expr1;
        std::shared_ptr<const Expression> _expr2;
    };

    class FunctionExpression : public Expression {
    public:
        explicit FunctionExpression(std::string func, std::vector<std::shared_ptr<const Expression>> args) : _func(std::move(func)), _args(std::move(args)) { }

        const std::string& getFunc() const { return _func; }
        const std::vector<std::shared_ptr<const Expression>>& getArgs() const { return _args; }

        virtual boost::variant<Value, std::shared_ptr<const Expression>> evaluate(const ExpressionContext& context) const override {
            std::vector<Value> vals;
            vals.reserve(_args.size());
            for (const std::shared_ptr<const Expression>& arg : _args) {
                boost::variant<Value, std::shared_ptr<const Expression>> result = arg->evaluate(context);
                if (auto val = boost::get<Value>(&result)) {
                    vals.push_back(*val);
                }
                else {
                    break;
                }
            }
            if (vals.size() == _args.size()) {
                try {
                    return applyFunc(_func, vals);
                }
                catch (const std::exception&) {
                    // if unknown function, keep it as expression
                }
            }
            std::vector<std::shared_ptr<const Expression>> exprs;
            exprs.reserve(_args.size());
            for (const std::shared_ptr<const Expression>& arg : _args) {
                boost::variant<Value, std::shared_ptr<const Expression>> result = arg->evaluate(context);
                if (auto val = boost::get<Value>(&result)) {
                    exprs.push_back(std::make_shared<ConstExpression>(*val));
                }
                else {
                    exprs.push_back(boost::get<std::shared_ptr<const Expression>>(result));
                }
            }
            return std::static_pointer_cast<const Expression>(std::make_shared<FunctionExpression>(_func, std::move(exprs)));
        }

        virtual bool equals(const Expression& expr) const override {
            if (auto funcExpr = dynamic_cast<const FunctionExpression*>(&expr)) {
                if (_func == funcExpr->_func && _args.size() == funcExpr->_args.size()) {
                    return std::equal(_args.begin(), _args.end(), funcExpr->_args.begin(), [](const std::shared_ptr<const Expression>& expr1, const std::shared_ptr<const Expression>& expr2) {
                        return expr1->equals(*expr2);
                    });
                }
            }
            return false;
        }
    private:
        static Color getColor(const Value& value) {
            if (auto color = boost::get<Color>(&value)) {
                return *color;
            }
            throw std::runtime_error("Wrong type, expecting color");
        }
        
        static float getFloat(const Value& value) {
            if (auto integer = boost::get<long long>(&value)) {
                return static_cast<float>(*integer);
            }
            else if (auto dbl = boost::get<double>(&value)) {
                return static_cast<float>(*dbl);
            }
            throw std::runtime_error("Wrong type, expecting float");
        }

        static Value applyFunc(const std::string& func, const std::vector<Value>& vals) {
            if (func == "url" && vals.size() == 1) {
                return Value(boost::lexical_cast<std::string>(vals[0]));
            }
            else if (func == "rgb" && vals.size() == 3) {
                std::array<float, 3> components;
                std::transform(vals.begin(), vals.end(), components.begin(), [](const Value& val) { return getFloat(val) / 255.0f; });
                Color color = Color::fromRGBA(components[0], components[1], components[2], 1.0f);
                return Value(color);
            }
            else if (func == "rgba" && vals.size() == 4) {
                std::array<float, 4> components;
                std::transform(vals.begin(), vals.begin() + 3, components.begin(), [](const Value& val) { return getFloat(val) / 255.0f; });
                components[3] = getFloat(vals[3]);
                Color color = Color::fromRGBA(components[0], components[1], components[2], components[3]);
                return Value(color);
            }
            else if (func == "mix" && vals.size() == 3) {
                Color color = Color::mix(getColor(vals[0]), getColor(vals[1]), getFloat(vals[2]));
                return Value(color);
            }
            else if (func == "lighten" && vals.size() == 2) {
                Color color = Color::lighten(getColor(vals[0]), getFloat(vals[1]));
                return Value(color);
            }
            else if (func == "darken" && vals.size() == 2) {
                Color color = Color::lighten(getColor(vals[0]), -getFloat(vals[1]));
                return Value(color);
            }
            else if (func == "saturate" && vals.size() == 2) {
                Color color = Color::saturate(getColor(vals[0]), getFloat(vals[1]));
                return Value(color);
            }
            else if (func == "desaturate" && vals.size() == 2) {
                Color color = Color::saturate(getColor(vals[0]), -getFloat(vals[1]));
                return Value(color);
            }
            else if (func == "fadein" && vals.size() == 2) {
                Color color = Color::fade(getColor(vals[0]), getFloat(vals[1]));
                return Value(color);
            }
            else if (func == "fadeout" && vals.size() == 2) {
                Color color = Color::fade(getColor(vals[0]), -getFloat(vals[1]));
                return Value(color);
            }
            throw std::runtime_error("Unsupported function/arity: " + func);
        }

        std::string _func;
        std::vector<std::shared_ptr<const Expression>> _args;
    };
} }

#endif
