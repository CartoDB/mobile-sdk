/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSS_CARTOCSSCOMPILER_H_
#define _CARTO_CARTOCSS_CARTOCSSCOMPILER_H_

#include "Expression.h"
#include "Predicate.h"
#include "StyleSheet.h"

#include <tuple>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <utility>

namespace carto { namespace css {
    class CartoCSSCompiler final {
    public:
        using RuleSpecificity = std::tuple<int, int, int, int>;

        struct Property {
            std::string field;
            std::shared_ptr<const Expression> expression;
            RuleSpecificity specificity;

            bool operator == (const Property& other) const {
                return field == other.field && expression == other.expression && specificity == other.specificity;
            }

            bool operator != (const Property& other) const {
                return !(*this == other);
            }
        };

        struct PropertySet {
            std::vector<std::shared_ptr<const Predicate>> filters;
            std::map<std::string, Property> properties;

            bool operator == (const PropertySet& other) const {
                return filters == other.filters && properties == other.properties;
            }

            bool operator != (const PropertySet& other) const {
                return !(*this == other);
            }
        };

        struct LayerAttachment {
            std::string attachment;
            int order = 0;
            std::list<PropertySet> propertySets;

            bool operator == (const LayerAttachment& other) const {
                return attachment == other.attachment && order == other.order && propertySets == other.propertySets;
            }

            bool operator != (const LayerAttachment& other) const {
                return !(*this == other);
            }
        };

        CartoCSSCompiler() = default;

        void setContext(const ExpressionContext& context) { _context = context; }
        void setIgnoreLayerPredicates(bool ignoreLayerPredicates) { _ignoreLayerPredicates = ignoreLayerPredicates; }

        void compileMap(const StyleSheet& styleSheet, std::map<std::string, Value>& mapProperties) const;
        void compileLayer(const std::string& layerName, const StyleSheet& styleSheet, std::list<LayerAttachment>& layerAttachments) const;
        
    private:
        struct FilteredProperty {
            Property property;
            std::vector<std::shared_ptr<const Predicate>> filters;
        };

        struct FilteredPropertyList {
            std::string attachment;
            std::list<FilteredProperty> properties;
        };

        void buildPropertyList(const RuleSet& ruleSet, const PredicateContext& context, const std::string& attachment, const std::vector<std::shared_ptr<const Predicate>>& filters, std::list<FilteredPropertyList>& propertyLists) const;
        
        static bool isRedundantPropertySet(std::list<PropertySet>::iterator begin, std::list<PropertySet>::iterator end, const PropertySet& propertySet);
        
        static RuleSpecificity calculateRuleSpecificity(const std::vector<std::shared_ptr<const Predicate>>& predicates, int order);

        ExpressionContext _context;
        bool _ignoreLayerPredicates = false;

        mutable std::map<Value, std::shared_ptr<const Expression>> _constCache;
        mutable std::map<std::shared_ptr<const Expression>, std::shared_ptr<const Expression>> _exprCache;
    };
} }

#endif
