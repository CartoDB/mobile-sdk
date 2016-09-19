/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_PREDICATEOPERATOR_H_
#define _CARTO_MAPNIKVT_PREDICATEOPERATOR_H_

#include "Predicate.h"
#include "ValueConverter.h"
#include "StringUtils.h"

namespace carto { namespace mvt {
    template <template <typename T> class Op, bool NullResult, bool MismatchResult>
    struct ComparisonOperator : public ComparisonPredicate::Operator {
        virtual bool apply(const Value& val1, const Value& val2) const override {
            return boost::apply_visitor(Operator(), val1, val2);
        }

    private:
        struct Operator : boost::static_visitor<bool> {
            bool operator() (boost::blank, boost::blank) const { return NullResult; }
            bool operator() (bool val1, long long val2) const { return Op<long long>()(static_cast<long long>(val1), val2); }
            bool operator() (bool val1, double val2) const { return Op<double>()(static_cast<double>(val1), val2); }
            bool operator() (long long val1, bool val2) const { return Op<long long>()(val1, static_cast<long long>(val2)); }
            bool operator() (long long val1, double val2) const { return Op<double>()(static_cast<double>(val1), val2); }
            bool operator() (double val1, bool val2) const { return Op<double>()(val1, static_cast<double>(val2)); }
            bool operator() (double val1, long long val2) const { return Op<double>()(val1, static_cast<double>(val2)); }
            template <typename T> bool operator() (T val1, T val2) const { return Op<T>()(val1, val2); }
            template <typename S, typename T> bool operator() (S val1, T val2) const { return MismatchResult; }
        };
    };

    using EQOperator = ComparisonOperator<std::equal_to, true, false>;

    using NEQOperator = ComparisonOperator<std::not_equal_to, false, true>;

    using GTOperator = ComparisonOperator<std::greater, false, false>;

    using GTEOperator = ComparisonOperator<std::greater_equal, true, false>;

    using LTOperator = ComparisonOperator<std::less, false, false>;

    using LTEOperator = ComparisonOperator<std::less_equal, true, false>;

    struct MatchOperator : public ComparisonPredicate::Operator {
        virtual bool apply(const Value& val1, const Value& val2) const override {
            std::string str1 = ValueConverter<std::string>::convert(val1);
            std::string str2 = ValueConverter<std::string>::convert(val2);
            return regexMatch(str1, str2);
        }
    };
} }

#endif
