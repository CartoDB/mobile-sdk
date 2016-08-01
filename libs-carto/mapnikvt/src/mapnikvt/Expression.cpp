#include "Expression.h"
#include "Predicate.h"

namespace carto { namespace mvt {
    Value PredicateExpression::evaluate(const ExpressionContext& context) const {
        return Value(_pred->evaluate(context));
    }

    void PredicateExpression::fold(std::function<void(const std::shared_ptr<const Expression>&)> fn) const {
        _pred->fold(fn);
    }
} }
