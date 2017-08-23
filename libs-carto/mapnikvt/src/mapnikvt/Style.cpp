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
        
        // Try to merge consecutive rules R1 and R2 with R1.maxZoom=R2.minZoom assuming everything else is equal
        for (auto it = _rules.begin(); it + 1 != _rules.end(); ) {
            std::shared_ptr<const Rule> rule1 = *(it + 0);
            std::shared_ptr<const Rule> rule2 = *(it + 1);
            if (rule1->getMinZoom() > rule2->getMinZoom()) {
                std::swap(rule1, rule2);
            }
            std::shared_ptr<const Predicate> pred1 = rule1->getFilter()->getPredicate();
            std::shared_ptr<const Predicate> pred2 = rule2->getFilter()->getPredicate();
            bool samePred = (pred1 == pred2) || (pred1 && pred2 && pred1->equals(pred2));
            if (rule1->getMaxZoom() == rule2->getMinZoom() && rule1->getFilter()->getType() == rule2->getFilter()->getType() && samePred && rule1->getSymbolizers().size() == rule2->getSymbolizers().size()) {
                if (std::equal(rule1->getSymbolizers().begin(), rule1->getSymbolizers().end(), rule2->getSymbolizers().begin(), [](const std::shared_ptr<Symbolizer>& symbolizer1, const std::shared_ptr<Symbolizer>& symbolizer2) {
                    return typeid(symbolizer1.get()) == typeid(symbolizer2.get()) && symbolizer1->getParameterMap() == symbolizer2->getParameterMap();
                })) {
                    auto combinedRule = std::make_shared<Rule>("combined", rule1->getMinZoom(), rule2->getMaxZoom(), rule1->getFilter(), rule1->getSymbolizers());
                    it = _rules.erase(it);
                    *it = combinedRule;
                    updated = true;
                    continue;
                }
            }
            it++;
        }

        // Try to merge consecutive rules R1 and R2 with different filter expressions but with everything else equal
        for (auto it = _rules.begin(); it + 1 != _rules.end(); ) {
            std::shared_ptr<const Rule> rule1 = *(it + 0);
            std::shared_ptr<const Rule> rule2 = *(it + 1);
            if (rule1->getMinZoom() == rule2->getMinZoom() && rule1->getMaxZoom() == rule2->getMaxZoom() && rule1->getFilter()->getType() == Filter::Type::FILTER && rule2->getFilter()->getType() == Filter::Type::FILTER && rule1->getSymbolizers().size() == rule2->getSymbolizers().size()) {
                if (std::equal(rule1->getSymbolizers().begin(), rule1->getSymbolizers().end(), rule2->getSymbolizers().begin(), [](const std::shared_ptr<Symbolizer>& symbolizer1, const std::shared_ptr<Symbolizer>& symbolizer2) {
                    return typeid(symbolizer1.get()) == typeid(symbolizer2.get()) && symbolizer1->getParameterMap() == symbolizer2->getParameterMap();
                })) {
                    auto combinedPred = buildOptimizedOrPredicate(rule1->getFilter()->getPredicate(), rule2->getFilter()->getPredicate());
                    auto combinedFilter = std::make_shared<Filter>(Filter::Type::FILTER, combinedPred);
                    auto combinedRule = std::make_shared<Rule>("combined", rule1->getMinZoom(), rule1->getMaxZoom(), combinedFilter, rule1->getSymbolizers());
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

    std::shared_ptr<const Predicate> Style::buildOptimizedOrPredicate(const std::shared_ptr<const Predicate>& pred1, const std::shared_ptr<const Predicate>& pred2) {
        // X or true = true
        if (!pred1 || !pred2) {
            return std::shared_ptr<Predicate>();
        }

        // X or X = X
        if (pred1->equals(pred2)) {
            return pred1;
        }

        // X or (X & Y) = X
        std::shared_ptr<const Predicate> preds[] = { pred1, pred2 };
        for (int i1 = 0; i1 < 2; i1++) {
            if (auto andPred2 = std::dynamic_pointer_cast<const AndPredicate>(preds[i1 ^ 1])) {
                std::shared_ptr<const Predicate> subPreds2[] = { andPred2->getPredicate1(), andPred2->getPredicate2() };
                for (int i2 = 0; i2 < 2; i2++) {
                    if (preds[i1]->equals(subPreds2[i2])) {
                        return preds[i1];
                    }
                }
            }
        }

        // (X & Y) or (X & Z) = X & (Y or Z)
        if (auto andPred1 = std::dynamic_pointer_cast<const AndPredicate>(pred1)) {
            std::shared_ptr<const Predicate> subPreds1[] = { andPred1->getPredicate1(), andPred1->getPredicate2() };
            if (auto andPred2 = std::dynamic_pointer_cast<const AndPredicate>(pred2)) {
                std::shared_ptr<const Predicate> subPreds2[] = { andPred2->getPredicate1(), andPred2->getPredicate2() };
                for (int i1 = 0; i1 < 2; i1++) {
                    for (int i2 = 0; i2 < 2; i2++) {
                        if (subPreds1[i1]->equals(subPreds2[i2])) {
                            auto subPred1 = subPreds1[i1];
                            auto subPred2 = std::make_shared<OrPredicate>(subPreds1[i1 ^ 1], subPreds2[i2 ^ 1]);
                            return std::make_shared<AndPredicate>(subPred1, subPred2);
                        }
                    }
                }
            }
        }

        // Just combine
        return std::make_shared<OrPredicate>(pred1, pred2);
    }
} }
