#include "Style.h"
#include "Expression.h"
#include "Filter.h"
#include "Predicate.h"
#include "Rule.h"
#include "Symbolizer.h"

#include <algorithm>

namespace carto { namespace mvt {
    Style::Style(std::string name, float opacity, std::string compOp, FilterMode filterMode, std::vector<std::shared_ptr<const Rule>> rules) : _name(std::move(name)), _opacity(opacity), _compOp(std::move(compOp)), _filterMode(filterMode), _rules(std::move(rules)) {
        rebuildZoomRuleMap();
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

    void Style::optimizeRules() {
        if (_filterMode != FilterMode::FIRST) {
            return;
        }
        if (_rules.size() < 1) {
            return;
        }

        bool updated = false;
        for (auto it = _rules.begin(); it + 1 != _rules.end(); ) {
            std::shared_ptr<const Rule> rule1 = *(it + 0);
            std::shared_ptr<const Rule> rule2 = *(it + 1);
            if (rule1->getMinZoom() == rule2->getMinZoom() && rule1->getMaxZoom() == rule2->getMaxZoom() && rule1->getFilter()->getType() == Filter::Type::FILTER && rule2->getFilter()->getType() == Filter::Type::FILTER && rule1->getSymbolizers().size() == rule2->getSymbolizers().size()) {
                if (std::equal(rule1->getSymbolizers().begin(), rule1->getSymbolizers().end(), rule2->getSymbolizers().begin(), [](const std::shared_ptr<Symbolizer>& symbolizer1, const std::shared_ptr<Symbolizer>& symbolizer2) {
                    return typeid(symbolizer1.get()) == typeid(symbolizer2.get()) && symbolizer1->getParameterMap() == symbolizer2->getParameterMap();
                })) {
                    std::shared_ptr<Predicate> combinedPred;
                    if (rule1->getFilter()->getPredicate() && rule2->getFilter()->getPredicate()) {
                        combinedPred = std::make_shared<OrPredicate>(rule1->getFilter()->getPredicate(), rule2->getFilter()->getPredicate());
                    }
                    std::shared_ptr<Filter> combinedFilter = std::make_shared<Filter>(Filter::Type::FILTER, combinedPred);
                    std::shared_ptr<Rule> combinedRule = std::make_shared<Rule>("combined", rule1->getMinZoom(), rule1->getMaxZoom(), combinedFilter, rule1->getSymbolizers());
                    it = _rules.erase(it);
                    *it = combinedRule;
                    updated = true;
                    continue;
                }
            }
            it++;
        }

        if (updated) {
            rebuildZoomRuleMap();
        }
    }

    void Style::rebuildZoomRuleMap() {
        _zoomRuleMap.clear();
        _zoomFieldExprsMap.clear();
        for (auto it = _rules.begin(); it != _rules.end(); it++) {
            const std::shared_ptr<const Rule>& rule = *it;
            std::unordered_set<std::shared_ptr<const Expression>> fieldExprs = rule->getReferencedFields();
            for (int zoom = rule->getMinZoom(); zoom < rule->getMaxZoom(); zoom++) {
                _zoomRuleMap[zoom].push_back(rule);
                _zoomFieldExprsMap[zoom].insert(fieldExprs.begin(), fieldExprs.end());
            }
        }
    }
} }
