/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_QUERYEXPRESSIONIMPL_H_
#define _CARTO_QUERYEXPRESSIONIMPL_H_

#include "search/query/QueryContext.h"
#include "search/query/QueryExpression.h"

#include <limits>
#include <regex>

#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>

#include <utf8.h>

namespace carto {
    namespace queryexpressionimpl {
        using Value = Variant;

        using Expression = QueryExpression;

        using Context = QueryContext;

        struct IsNullPredicate {
            bool operator() (const Value& val) const { return val.getType() == VariantType::VARIANT_TYPE_NULL; }
        };

        struct IsNotNullPredicate {
            bool operator() (const Value& val) const { return val.getType() != VariantType::VARIANT_TYPE_NULL; }
        };

        struct RegexpLikePredicate {
            bool operator() (const Value& val1, const Value& val2) const {
                switch (val1.getType()) {
                case VariantType::VARIANT_TYPE_NULL:
                case VariantType::VARIANT_TYPE_ARRAY:
                case VariantType::VARIANT_TYPE_OBJECT:
                    return false;
                default:
                    break;
                }
                std::string str = val1.getString();
                std::wstring wstr;
                utf8::utf8to32(str.begin(), str.end(), std::back_inserter(wstr));

                switch (val2.getType()) {
                case VariantType::VARIANT_TYPE_NULL:
                case VariantType::VARIANT_TYPE_ARRAY:
                case VariantType::VARIANT_TYPE_OBJECT:
                    return false;
                default:
                    break;
                }
                std::string re = val2.getString();
                std::wstring wre;
                utf8::utf8to32(re.begin(), re.end(), std::back_inserter(wre));

                return std::regex_match(wstr, std::wregex(wre));
            }
        };

        struct EqPredicate {
            bool operator() (const Value& val1, const Value& val2) const {
                if (val1.getType() == VariantType::VARIANT_TYPE_NULL || val2.getType() == VariantType::VARIANT_TYPE_NULL) {
                    return false;
                }
                return val1.toPicoJSON() == val2.toPicoJSON();
            }
        };

        struct NeqPredicate {
            bool operator() (const Value& val1, const Value& val2) const {
                if (val1.getType() == VariantType::VARIANT_TYPE_NULL || val2.getType() == VariantType::VARIANT_TYPE_NULL) {
                    return false;
                }
                return val1.toPicoJSON() != val2.toPicoJSON();
            }
        };

        template <template <typename T> class Op>
        struct ComparisonPredicate {
            bool operator() (const Value& val1, const Value& val2) const {
                switch (val1.getType()) {
                case VariantType::VARIANT_TYPE_NULL:
                case VariantType::VARIANT_TYPE_ARRAY:
                case VariantType::VARIANT_TYPE_OBJECT:
                    return false;
                default:
                    break;
                }

                switch (val2.getType()) {
                case VariantType::VARIANT_TYPE_NULL:
                case VariantType::VARIANT_TYPE_ARRAY:
                case VariantType::VARIANT_TYPE_OBJECT:
                    return false;
                default:
                    break;
                }

                const picojson::value& v1 = val1.toPicoJSON();
                const picojson::value& v2 = val2.toPicoJSON();
                if (v1.is<bool>() && v2.is<bool>()) {
                    return Op<bool>()(v1.get<bool>(), v2.get<bool>());
                }
                if (v1.is<double>() && v2.is<double>()) { // NOTE: works for integers also
                    if (v1.is<std::int64_t>() && v2.is<std::int64_t>()) {
                        return Op<std::int64_t>()(v1.get<std::int64_t>(), v2.get<std::int64_t>());
                    }
                    return Op<double>()(v1.get<double>(), v2.get<double>());
                }
                if (v1.is<std::string>() && v2.is<std::string>()) {
                    std::string str1 = v1.get<std::string>();
                    std::wstring wstr1;
                    utf8::utf8to32(str1.begin(), str1.end(), std::back_inserter(wstr1));
                    std::string str2 = v2.get<std::string>();
                    std::wstring wstr2;
                    utf8::utf8to32(str2.begin(), str2.end(), std::back_inserter(wstr2));
                    return Op<std::wstring>()(wstr1, wstr2);
                }
                return false;
            }
        };


        using GtPredicate = ComparisonPredicate<std::greater>;
        using LtPredicate = ComparisonPredicate<std::less>;

        struct GtePredicate {
            bool operator() (const Value& val1, const Value& val2) const {
                return EqPredicate()(val1, val2) || GtPredicate()(val1, val2);
            }
        };

        struct LtePredicate {
            bool operator() (const Value& val1, const Value& val2) const {
                return EqPredicate()(val1, val2) || LtPredicate()(val1, val2);
            }
        };

        struct Operand {
            virtual ~Operand() = default;
            virtual Value evaluate(const Context& context) const = 0;
        };

        struct ConstOperand : public Operand {
            explicit ConstOperand(const Value& value) : _value(value) { }
            virtual Value evaluate(const Context& context) const { return _value; }
            static std::shared_ptr<ConstOperand> create(const Value& value) { return std::make_shared<ConstOperand>(value); }
        private:
            Value _value;
        };

        struct VariableOperand : public Operand {
            VariableOperand(const std::string& name, bool nocase) : _name(name), _nocase(nocase) { }

            virtual Value evaluate(const Context& context) const {
                Variant value;
                if (!context.getVariable(_name, value)) {
                    return Value();
                }
                if (_nocase && value.getType() == VariantType::VARIANT_TYPE_STRING) {
                    value = Value(CollateNoCase(value.getString()));
                }
                return value;
            }

            static std::shared_ptr<VariableOperand> create(const std::string& name) { return std::make_shared<VariableOperand>(name, false); }
            static std::shared_ptr<VariableOperand> createEx(const std::string& name, const std::string& collateSeq) { return std::make_shared<VariableOperand>(name, CollateNoCase(collateSeq) == CollateNoCase("nocase")); }

        private:
            static std::string CollateNoCase(std::string str) {
                for (std::size_t i = 0; i < str.size(); i++) {
                    // TODO: proper unicode conversion
                    str[i] = (str[i] >= 'A' && str[i] <= 'Z' ? str[i] - 'A' + 'a' : str[i]);
                }
                return str;
            }

            std::string _name;
            bool _nocase;
        };

        struct NotExpression : public Expression {
            explicit NotExpression(const std::shared_ptr<Expression>& expr) : _expr(expr) { }
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
