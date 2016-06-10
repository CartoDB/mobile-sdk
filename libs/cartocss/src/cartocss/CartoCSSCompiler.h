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
    class CartoCSSCompiler {
    public:
        struct Property {
            using Specificity = std::tuple<int, int, int, int>;
            
            std::string field;
            std::shared_ptr<const Expression> expression;
            Specificity specificity;
        };

        struct PropertySet {
            std::vector<std::shared_ptr<const Predicate>> filters;
            std::map<std::string, Property> properties;
        };

        struct LayerAttachment {
            std::string attachment;
            int order = 0;
            std::list<PropertySet> propertySets;
        };
        
        explicit CartoCSSCompiler(const ExpressionContext& context, bool ignoreLayerPredicates) : _context(context), _ignoreLayerPredicates(ignoreLayerPredicates) { }

        void compileMap(const StyleSheet& styleSheet, std::map<std::string, Value>& mapProperties) const;
        
        void compileLayer(const std::string& layerName, const StyleSheet& styleSheet, std::list<LayerAttachment>& layerAttachments) const;
        
    private:
        struct FilteredProperty : Property {
            std::vector<std::shared_ptr<const Predicate>> filters;
        };

        struct FilteredPropertyList {
            std::string attachment;
            std::list<FilteredProperty> properties;
        };

        void buildPropertyList(const RuleSet& ruleSet, const PredicateContext& context, const std::string& attachment, const std::vector<std::shared_ptr<const Predicate>>& filters, std::list<FilteredPropertyList>& propertyLists) const;
        
        static bool isRedundantPropertySet(std::list<PropertySet>::iterator begin, std::list<PropertySet>::iterator end, const PropertySet& propertySet);
        
        static Property::Specificity calculateSpecificity(const std::vector<std::shared_ptr<const Predicate>>& predicates, int order);

        bool _ignoreLayerPredicates;

        ExpressionContext _context;
    };
} }

#endif
