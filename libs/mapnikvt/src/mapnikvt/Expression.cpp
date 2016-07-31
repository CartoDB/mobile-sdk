#include "Expression.h"
#include "Predicate.h"
#include "ParserUtils.h"
#include "GeneratorUtils.h"

#include <algorithm>
#include <sstream>

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

    InterpolateExpression::InterpolateExpression(Method method, std::shared_ptr<const Expression> timeExpr, std::vector<Value> keyFrames) : _method(method), _timeExpr(std::move(timeExpr)), _keyFrames(std::move(keyFrames)), _fcurve() {
        cglib::fcurve_type type = cglib::fcurve_type::linear;
        switch (_method) {
        case Method::STEP:
            type = cglib::fcurve_type::step;
            break;
        case Method::LINEAR:
            type = cglib::fcurve_type::linear;
            break;
        case Method::CUBIC:
            type = cglib::fcurve_type::cubic;
            break;
        }

        std::vector<cglib::vec2<float>> keyFramesList;
        for (std::size_t i = 0; i + 1 < _keyFrames.size(); i += 2) {
            keyFramesList.emplace_back(ValueConverter<float>::convert(_keyFrames[i + 0]), ValueConverter<float>::convert(_keyFrames[i + 1]));
        }

        _fcurve = cglib::fcurve<cglib::vec2<float>>::create(type, keyFramesList.begin(), keyFramesList.end());
    }

    Value InterpolateExpression::evaluate(const ExpressionContext& context) const {
        float time = ValueConverter<float>::convert(_timeExpr->evaluate(context));
        float val = _fcurve.evaluate(time)(1);
        return Value(val);
    }

    bool InterpolateExpression::equals(const std::shared_ptr<const Expression>& expr) const {
        if (auto interpolateExpr = std::dynamic_pointer_cast<const InterpolateExpression>(expr)) {
            return interpolateExpr->_method == _method && interpolateExpr->_timeExpr->equals(_timeExpr) && interpolateExpr->_keyFrames == _keyFrames;
        }
        return false;
    }

    std::shared_ptr<const Expression> InterpolateExpression::map(std::function<std::shared_ptr<const Expression>(const std::shared_ptr<const Expression>&)> fn) const {
        std::shared_ptr<const Expression> timeExpr = _timeExpr->map(fn);
        return fn(std::make_shared<InterpolateExpression>(_method, timeExpr, _keyFrames));
    }

    void InterpolateExpression::fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const {
        fn(shared_from_this());
        fn(_timeExpr);
    }
} }
