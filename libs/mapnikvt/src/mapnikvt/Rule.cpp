#include "Rule.h"
#include "Symbolizer.h"
#include "Filter.h"
#include "Expression.h"
#include "Predicate.h"

namespace carto { namespace mvt {
    std::unordered_set<std::shared_ptr<const Expression>> Rule::getReferencedFields() const {
        std::unordered_set<std::shared_ptr<const Expression>> fieldExprs;
        auto gatherFields = [&](const std::shared_ptr<const Expression>& expr) {
            if (auto varExpr = std::dynamic_pointer_cast<const VariableExpression>(expr)) {
                fieldExprs.insert(varExpr->getVariableExpression());
            }
        };
        if (_filter) {
            if (auto pred = _filter->getPredicate()) {
                pred->fold(gatherFields);
            }
        }
        std::for_each(_symbolizers.begin(), _symbolizers.end(), [&](const std::shared_ptr<Symbolizer>& symbolizer) {
            for (const std::shared_ptr<const Expression>& expr : symbolizer->getParameterExpressions()) {
                expr->fold(gatherFields);
            }
        });
        return fieldExprs;
    }
} }