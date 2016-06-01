/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_EXPRESSIONOPERATOR_H_
#define _CARTO_MAPNIKVT_EXPRESSIONOPERATOR_H_

#include "Expression.h"
#include "StringUtils.h"
#include "ValueConverter.h"

namespace carto { namespace mvt {
    struct LengthOperator : public UnaryExpression::Operator {
        virtual Value apply(const Value& val) const override {
            std::string str = ValueConverter<std::string>::convert(val);
            return Value(static_cast<long long>(str.size()));
        }
    };

    struct UpperCaseOperator : public UnaryExpression::Operator {
        virtual Value apply(const Value& val) const override {
            std::string str = ValueConverter<std::string>::convert(val);
            return Value(toUpper(str));
        }
    };

    struct LowerCaseOperator : public UnaryExpression::Operator {
        virtual Value apply(const Value& val) const override {
            std::string str = ValueConverter<std::string>::convert(val);
            return Value(toLower(str));
        }
    };

    struct CapitalizeOperator : public UnaryExpression::Operator {
        virtual Value apply(const Value& val) const override {
            std::string str = ValueConverter<std::string>::convert(val);
            return Value(capitalize(str));
        }
    };

    struct NegOperator : public UnaryExpression::Operator {
        virtual Value apply(const Value& val) const override {
            return boost::apply_visitor(Operator(), val);
        }

    private:
        struct Operator : boost::static_visitor<Value> {
            Value operator() (bool val) const { return Value(-static_cast<long long>(val)); }
            Value operator() (long long val) const { return Value(-val); }
            Value operator() (double val) const { return Value(-val); }
            template <typename T> Value operator() (T val) const { return Value(val); }
        };
    };

    template <template <typename T> class Op>
    struct ArithmeticOperator : public BinaryExpression::Operator {
        virtual Value apply(const Value& val1, const Value& val2) const override {
            return boost::apply_visitor(Operator(), val1, val2);
        }

    protected:
        struct Operator : boost::static_visitor<Value> {
            Value operator() (bool val1, bool val2) const { return Value(Op<long long>()(static_cast<long long>(val1), static_cast<long long>(val2))); }
            Value operator() (long long val1, long long val2) const { return Value(Op<long long>()(val1, val2)); }
            Value operator() (long long val1, double val2) const { return Value(Op<double>()(static_cast<double>(val1), val2)); }
            Value operator() (double val1, long long val2) const { return Value(Op<double>()(val1, static_cast<double>(val2))); }
            Value operator() (double val1, double val2) const { return Value(Op<double>()(val1, val2)); }
            template <typename S, typename T> Value operator() (S val1, T val2) const { return Value(val1); }
        };
    };

    struct AddOperator : ArithmeticOperator<std::plus> {
        virtual Value apply(const Value& val1, const Value& val2) const override {
            if (boost::get<std::string>(&val1) || boost::get<std::string>(&val2)) {
                std::string str1 = ValueConverter<std::string>::convert(val1);
                std::string str2 = ValueConverter<std::string>::convert(val2);
                return Value(str1 + str2);
            }
            return boost::apply_visitor(Operator(), val1, val2);
        }
    };

    using SubOperator = ArithmeticOperator<std::minus>;

    using MulOperator = ArithmeticOperator<std::multiplies>;

    struct DivOperator : ArithmeticOperator<std::divides> {
        virtual Value apply(const Value& val1, const Value& val2) const override {
            return boost::apply_visitor(Operator(), val1, val2);
        }

    private:
        struct Operator : boost::static_visitor<Value> {
            Value operator() (long long val1, long long val2) const { return val2 != 0 ? Value(val1 / val2) : Value(); }
            Value operator() (long long val1, double val2) const { return Value(static_cast<double>(val1) / val2); }
            Value operator() (double val1, long long val2) const { return Value(val1 / static_cast<double>(val2)); }
            Value operator() (double val1, double val2) const { return Value(val1 / val2); }
            template <typename S, typename T> Value operator() (S val1, T val2) const { return Value(val1); }
        };
    };

    struct ModOperator : public ArithmeticOperator<std::divides> {
        virtual Value apply(const Value& val1, const Value& val2) const override {
            return boost::apply_visitor(Operator(), val1, val2);
        }

    private:
        struct Operator : boost::static_visitor<Value> {
            Value operator() (long long val1, long long val2) const { return val2 != 0 ? Value(val1 % val2) : Value(); }
            Value operator() (long long val1, double val2) const { return Value(std::fmod(static_cast<double>(val1), val2)); }
            Value operator() (double val1, long long val2) const { return Value(std::fmod(val1, static_cast<double>(val2))); }
            Value operator() (double val1, double val2) const { return Value(std::fmod(val1, val2)); }
            template <typename S, typename T> Value operator() (S val1, T val2) const { return Value(val1); }
        };
    };

    struct ConcatenateOperator : public BinaryExpression::Operator {
        virtual Value apply(const Value& val1, const Value& val2) const override {
            std::string str1 = ValueConverter<std::string>::convert(val1);
            std::string str2 = ValueConverter<std::string>::convert(val2);
            return Value(str1.append(str2));
        }
    };

    struct ReplaceOperator : public TertiaryExpression::Operator {
        virtual Value apply(const Value& val1, const Value& val2, const Value& val3) const override {
            std::string str1 = ValueConverter<std::string>::convert(val1);
            std::string str2 = ValueConverter<std::string>::convert(val2);
            std::string str3 = ValueConverter<std::string>::convert(val3);
            return Value(regexReplace(str1, str2, str3));
        }
    };

    struct ConditionalOperator : public TertiaryExpression::Operator {
        virtual Value apply(const Value& val1, const Value& val2, const Value& val3) const override {
            bool b1 = ValueConverter<bool>::convert(val1);
            return b1 ? val2 : val3;
        }
    };
} }

#endif
