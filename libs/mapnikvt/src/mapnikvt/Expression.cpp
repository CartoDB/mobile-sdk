#include "Expression.h"
#include "Predicate.h"

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
} }
