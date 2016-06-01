#include "CartoCSSCompiler.h"

#include <set>

namespace carto { namespace css {
    void CartoCSSCompiler::compileMap(const StyleSheet& styleSheet, std::map<std::string, Value>& mapProperties) const {
        std::map<std::string, std::shared_ptr<const Expression>> variableMap;
        PredicateContext context;
        context.expressionContext = _context;
        context.expressionContext.variableMap = &variableMap;
        std::list<FilteredPropertyList> propertyLists;
        for (const StyleSheet::Element& element : styleSheet.getElements()) {
            if (auto decl = boost::get<VariableDeclaration>(&element)) {
                if (variableMap.find(decl->getVariable()) == variableMap.end()) {
                    variableMap[decl->getVariable()] = decl->getExpression();
                }
            }
            else if (auto ruleSet = boost::get<RuleSet>(&element)) {
                buildPropertyList(*ruleSet, context, "", std::vector<std::shared_ptr<const Predicate>>(), propertyLists);
            }
        }

        for (const FilteredPropertyList& propertyList : propertyLists) {
            if (propertyList.attachment.empty()) {
                for (const FilteredProperty& prop : propertyList.properties) {
                    if (prop.filters.empty()) {
                        boost::variant<Value, std::shared_ptr<const Expression>> propResult = prop.expression->evaluate(context.expressionContext);
                        if (auto val = boost::get<Value>(&propResult)) {
                            mapProperties[prop.field] = *val;
                        }
                    }
                }
            }
        }
    }
    
    void CartoCSSCompiler::compileLayer(const std::string& layerName, const StyleSheet& styleSheet, std::list<LayerAttachment>& layerAttachments) const {
        std::map<std::string, std::shared_ptr<const Expression>> variableMap;
        PredicateContext context;
        context.layerName = layerName;
        context.expressionContext = _context;
        context.expressionContext.variableMap = &variableMap;
        std::list<FilteredPropertyList> propertyLists;
        for (const StyleSheet::Element& element : styleSheet.getElements()) {
            if (auto decl = boost::get<VariableDeclaration>(&element)) {
                if (variableMap.find(decl->getVariable()) == variableMap.end()) {
                    variableMap[decl->getVariable()] = decl->getExpression();
                }
            } else if (auto ruleSet = boost::get<RuleSet>(&element)) {
                buildPropertyList(*ruleSet, context, "", std::vector<std::shared_ptr<const Predicate>>(), propertyLists);
            }
        }
        
        for (FilteredPropertyList& propertyList : propertyLists) {
            // Sort the properties by decreasing specificity
            propertyList.properties.sort([&](const Property& prop1, const Property& prop2) {
                return prop1.specificity > prop2.specificity;
            });
            
            // Build property set list
            std::list<PropertySet> propertySets;
            for (FilteredProperty prop : propertyList.properties) {
                // Try to evaluate property expression, store the result as expression (even when constant)
                boost::variant<Value, std::shared_ptr<const Expression>> propResult = prop.expression->evaluate(context.expressionContext);
                if (auto val = boost::get<Value>(&propResult)) {
                    prop.expression = std::make_shared<ConstExpression>(*val);
                }
                else {
                    prop.expression = boost::get<std::shared_ptr<const Expression>>(propResult);
                }
                
                for (auto propertySetIt = propertySets.begin(); propertySetIt != propertySets.end(); propertySetIt++) {
                    // Check if this attribute is already set for given property set
                    auto attributeIt = propertySetIt->properties.find(prop.field);
                    if (attributeIt != propertySetIt->properties.end()) {
                        if (attributeIt->second.specificity >= prop.specificity) {
                            continue;
                        }
                        if (attributeIt->second.expression->equals(*prop.expression)) {
                            continue;
                        }
                    }

                    // Build new property set by setting the attribute and combining filters
                    PropertySet propertySet(*propertySetIt);
                    propertySet.properties[prop.field] = prop;
                    bool skip = false;
                    for (const std::shared_ptr<const Predicate>& propFilter : prop.filters) {
                        auto filterIt = propertySet.filters.begin();
                        for (; filterIt != propertySet.filters.end(); filterIt++) {
                            // If the filter contains existing filter, then done
                            if (propFilter == *filterIt || propFilter->contains(*filterIt)) {
                                break;
                            }

                            // Not possible to satisfy both filters? Skip this combination in that case
                            if (!propFilter->intersects(*filterIt)) {
                                skip = true;
                                break;
                            }
                        }
                        if (filterIt == propertySet.filters.end()) {
                            propertySet.filters.push_back(propFilter);
                        }
                    }
                    if (skip) {
                        continue;
                    }

                    // If filters did not change, replace existing filter otherwise we must insert the new filter and keep old one
                    if (isRedundantPropertySet(propertySets.begin(), propertySetIt, propertySet)) {
                        continue;
                    }
                    if (propertySet.filters == propertySetIt->filters) {
                        *propertySetIt = propertySet;
                    }
                    else {
                        propertySets.insert(propertySetIt, propertySet);
                    }
                }

                // Add the property set
                PropertySet propertySet;
                propertySet.properties[prop.field] = prop;
                propertySet.filters = prop.filters;
                if (isRedundantPropertySet(propertySets.begin(), propertySets.end(), propertySet)) {
                    continue;
                }
                propertySets.push_back(std::move(propertySet));
            }
            
            // Add layer attachment
            LayerAttachment layerAttachment;
            layerAttachment.attachment = propertyList.attachment;
            layerAttachment.order = std::numeric_limits<int>::max();
            for (const PropertySet& propertySet : propertySets) {
                for (const std::pair<std::string, Property>& namedProp : propertySet.properties) {
                    layerAttachment.order = std::min(layerAttachment.order, std::get<3>(namedProp.second.specificity));
                }
            }
            layerAttachment.propertySets = propertySets;
            layerAttachments.push_back(std::move(layerAttachment));
        }
    }
    
    void CartoCSSCompiler::buildPropertyList(const RuleSet& ruleSet, const PredicateContext& context, const std::string& attachment, const std::vector<std::shared_ptr<const Predicate>>& filters, std::list<FilteredPropertyList>& propertyLists) const {
        // List of selectors to use
        std::vector<Selector> selectors = ruleSet.getSelectors();
        if (selectors.empty() && !context.layerName.empty()) {
            selectors.push_back(Selector()); // special case, empty selector list means 'all'
        }
        
        // Process all selectors
        for (const Selector& selector : selectors) {
            // Build filters for given selector
            std::vector<std::shared_ptr<const Predicate>> selectorFilters(filters);
            selectorFilters.insert(selectorFilters.end(), selector.getPredicates().begin(), selector.getPredicates().end());
            
            // Check if the filter list is 'reachable'. Also, remove always true conditions from the filter
            bool unreachable = false;
            std::string selectorAttachment = attachment;
            std::vector<std::shared_ptr<const Predicate>> optimizedSelectorFilters;
            for (auto it = selectorFilters.begin(); it != selectorFilters.end(); ) {
                boost::tribool pred = (*it)->evaluate(context);
                if (!pred) {
                    unreachable = true;
                    break;
                }
                if (auto attachmentPred = std::dynamic_pointer_cast<const AttachmentPredicate>(*it)) {
                    selectorAttachment += "::" + attachmentPred->getAttachment();
                    it = selectorFilters.erase(it);
                    continue;
                }
                if (boost::indeterminate(pred)) { // ignore always true filters
                    optimizedSelectorFilters.push_back(*it);
                }
                it++;
            }
            if (unreachable) {
                continue;
            }
            
            // Process block elements
            std::set<std::string> blockFields;
            for (const Block::Element& element : ruleSet.getBlock().getElements()) {
                if (auto decl = boost::get<PropertyDeclaration>(&element)) {
                    if (blockFields.find(decl->getField()) != blockFields.end()) {
                        continue;
                    }
                    blockFields.insert(decl->getField());
                    
                    // Find property set list for current attachment
                    auto propertyListsIt = std::find_if(propertyLists.begin(), propertyLists.end(), [&](const FilteredPropertyList& propertyList) { return propertyList.attachment == selectorAttachment; });
                    if (propertyListsIt == propertyLists.end()) {
                        FilteredPropertyList propertyList;
                        propertyList.attachment = selectorAttachment;
                        propertyListsIt = propertyLists.insert(propertyListsIt, propertyList);
                    }
                    std::list<FilteredProperty>& properties = (*propertyListsIt).properties;
                    
                    // Add property
                    FilteredProperty prop;
                    prop.filters = optimizedSelectorFilters;
                    prop.field = decl->getField();
                    prop.expression = decl->getExpression();
                    prop.specificity = calculateSpecificity(selectorFilters, decl->getOrder());
                    properties.push_back(prop);
                } else if (auto subRuleSet = boost::get<RuleSet>(&element)) {
                    // Recurse with subrule
                    buildPropertyList(*subRuleSet, context, selectorAttachment, selectorFilters, propertyLists);
                }
            }
        }
    }
    
    bool CartoCSSCompiler::isRedundantPropertySet(std::list<PropertySet>::iterator begin, std::list<PropertySet>::iterator end, const PropertySet& propertySet) {
        for (auto it = begin; it != end; it++) {
            if (std::all_of(it->filters.begin(), it->filters.end(), [&](const std::shared_ptr<const Predicate>& filter) {
                if (std::find(propertySet.filters.begin(), propertySet.filters.end(), filter) != propertySet.filters.end()) {
                    return true;
                }
                return std::any_of(propertySet.filters.begin(), propertySet.filters.end(), [&](const std::shared_ptr<const Predicate>& propFilter) {
                    return filter->contains(propFilter);
                });
            })) {
                return true;
            }
        }
        return false;
    }
    
    CartoCSSCompiler::Property::Specificity CartoCSSCompiler::calculateSpecificity(const std::vector<std::shared_ptr<const Predicate>>& predicates, int order) {
        int layers = 0;
        int classes = 0;
        int filters = 0;
        for (const std::shared_ptr<const Predicate>& pred : predicates) {
            if (std::dynamic_pointer_cast<const LayerPredicate>(pred)) {
                layers++;
            } else if (std::dynamic_pointer_cast<const ClassPredicate>(pred)) {
                classes++;
            } else if (std::dynamic_pointer_cast<const AttachmentPredicate>(pred)) {
                // ignore
            } else {
                filters++;
            }
        }
        return std::make_tuple(layers, classes, filters, order);
    }
} }
