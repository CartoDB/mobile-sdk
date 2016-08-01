#include "Style.h"
#include "Expression.h"
#include "Rule.h"

namespace carto { namespace mvt {
    Style::Style(std::string name, float opacity, std::string compOp, FilterMode filterMode, std::vector<std::shared_ptr<const Rule>> rules) : _name(std::move(name)), _opacity(opacity), _compOp(std::move(compOp)), _filterMode(filterMode), _rules(std::move(rules)), _zoomRuleMap() {
        for (auto it = _rules.begin(); it != _rules.end(); it++) {
            const std::shared_ptr<const Rule>& rule = *it;
            std::unordered_set<std::shared_ptr<const Expression>> fieldExprs = rule->getReferencedFields();
            for (int zoom = rule->getMinZoom(); zoom < rule->getMaxZoom(); zoom++) {
                _zoomRuleMap[zoom].push_back(rule);
                _zoomFieldExprsMap[zoom].insert(fieldExprs.begin(), fieldExprs.end());
            }
        }
    }

    const std::vector<std::shared_ptr<const Rule>>& Style::getZoomRules(int zoom) const {
        static const std::vector<std::shared_ptr<const Rule>> emptyRules;
        auto it = _zoomRuleMap.find(zoom);
        if (it == _zoomRuleMap.end()) {
            return emptyRules;
        }
        return it->second;
    }

    const std::unordered_set<std::shared_ptr<const Expression>>& Style::getReferencedFields(int zoom) const {
        static const std::unordered_set<std::shared_ptr<const Expression>> emptyFieldExprs;
        auto it = _zoomFieldExprsMap.find(zoom);
        if (it == _zoomFieldExprsMap.end()) {
            return emptyFieldExprs;
        }
        return it->second;
    }
} }
