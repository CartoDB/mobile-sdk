#include "Expression.h"
#include "Predicate.h"

#include <algorithm>

#include <boost/algorithm/string.hpp>

#include <cglib/fcurve.h>

namespace carto { namespace mvt {
    Value PredicateExpression::evaluate(const ExpressionContext& context) const {
        return Value(_pred->evaluate(context));
    }

    bool PredicateExpression::equals(const std::shared_ptr<const Expression>& expr) const {
        if (auto predExpr = std::dynamic_pointer_cast<const PredicateExpression>(expr)) {
            return predExpr->_pred->equals(_pred);
        }
        return false;
    }

    std::shared_ptr<const Expression> PredicateExpression::map(std::function<std::shared_ptr<const Expression>(const std::shared_ptr<const Expression>&)> fn) const {
        std::shared_ptr<const Predicate> pred = _pred->map(fn);
        return fn(std::make_shared<PredicateExpression>(pred));
    }

    void PredicateExpression::fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const {
        _pred->fold(fn);
    }

    Value InterpolateExpression::evaluate(const ExpressionContext& context) const {
        float time = ValueConverter<float>::convert(_timeExpr->evaluate(context));
        std::vector<cglib::vec2<float>> keyFrames;
        for (const std::shared_ptr<const Expression>& keyFrameExpr : _keyFrameExprs) {
            std::vector<std::string> keyValue;
            boost::split(keyValue, ValueConverter<std::string>::convert(keyFrameExpr->evaluate(context)), boost::is_any_of(","));
            if (keyValue.size() == 2) {
                float key = boost::lexical_cast<float>(keyValue[0]);
                float val = boost::lexical_cast<float>(keyValue[1]);
                keyFrames.emplace_back(key, val);
            }
        }
        cglib::fcurve<cglib::vec2<float>>::curve_type type = cglib::fcurve<cglib::vec2<float>>::linear_curve;
        switch (_method)
        {
        case Method::STEP:
            type = cglib::fcurve<cglib::vec2<float>>::step_curve;
            break;
        case Method::LINEAR:
            type = cglib::fcurve<cglib::vec2<float>>::linear_curve;
            break;
        case Method::CUBIC:
            type = cglib::fcurve<cglib::vec2<float>>::cubic_curve;
            break;
        }
        cglib::fcurve<cglib::vec2<float>> curve = cglib::fcurve<cglib::vec2<float>>::create(type, keyFrames.begin(), keyFrames.end());
        float val = curve.evaluate(time)(1);
        return Value(val);
    }

    bool InterpolateExpression::equals(const std::shared_ptr<const Expression>& expr) const {
        if (auto interpolateExpr = std::dynamic_pointer_cast<const InterpolateExpression>(expr)) {
            if (!(interpolateExpr->_method == _method && interpolateExpr->_timeExpr->equals(_timeExpr) && interpolateExpr->_keyFrameExprs.size() == _keyFrameExprs.size())) {
                return false;
            }
            for (std::size_t i = 0; i < _keyFrameExprs.size(); i++) {
                if (!interpolateExpr->_keyFrameExprs[i]->equals(_keyFrameExprs[i])) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    std::shared_ptr<const Expression> InterpolateExpression::map(std::function<std::shared_ptr<const Expression>(const std::shared_ptr<const Expression>&)> fn) const {
        std::shared_ptr<const Expression> timeExpr = _timeExpr->map(fn);
        std::vector<std::shared_ptr<const Expression>> keyFrameExprs;
        std::transform(_keyFrameExprs.begin(), _keyFrameExprs.end(), std::back_inserter(keyFrameExprs), [&fn](const std::shared_ptr<const Expression>& expr) {
            return fn(expr);
        });
        return fn(std::make_shared<InterpolateExpression>(_method, timeExpr, keyFrameExprs));
    }

    void InterpolateExpression::fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const {
        fn(shared_from_this());
        fn(_timeExpr);
        for (const std::shared_ptr<const Expression>& expr : _keyFrameExprs) {
            fn(expr);
        }
    }
} }
