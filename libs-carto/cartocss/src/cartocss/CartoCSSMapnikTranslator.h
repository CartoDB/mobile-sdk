/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSS_CARTOCSSMAPNIKTRANSLATOR_H_
#define _CARTO_CARTOCSS_CARTOCSSMAPNIKTRANSLATOR_H_

#include "Expression.h"
#include "Predicate.h"
#include "CartoCSSCompiler.h"
#include "mapnikvt/Map.h"
#include "mapnikvt/Rule.h"
#include "mapnikvt/Expression.h"
#include "mapnikvt/Predicate.h"
#include "mapnikvt/Logger.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace carto { namespace css {
    class CartoCSSMapnikTranslator {
    public:
        class TranslatorException : public std::runtime_error {
        public:
            explicit TranslatorException(const std::string& msg) : runtime_error(msg) { }
        };

        explicit CartoCSSMapnikTranslator(std::shared_ptr<mvt::Logger> logger) : _logger(std::move(logger)) { }
        virtual ~CartoCSSMapnikTranslator() = default;

        virtual std::shared_ptr<mvt::Rule> buildRule(const CartoCSSCompiler::PropertySet& propertySet, const std::shared_ptr<mvt::Map>& map, int zoom) const;

        virtual std::shared_ptr<mvt::Symbolizer> buildSymbolizer(const std::string& symbolizerType, const std::list<CartoCSSCompiler::Property>& properties, const std::shared_ptr<mvt::Map>& map, int zoom) const;

        virtual std::string buildExpressionString(const std::shared_ptr<const Expression>& expr, bool stringExpr) const;

        virtual std::string buildFunctionExpressionString(const std::shared_ptr<const FunctionExpression>& funcExpr, bool topLevel) const;

        virtual std::shared_ptr<mvt::Predicate> buildPredicate(const std::shared_ptr<const Predicate>& pred) const;

        virtual std::shared_ptr<mvt::ComparisonPredicate::Operator> buildOperator(OpPredicate::Op op) const;

        virtual mvt::Value buildValue(const Value& val) const;

    protected:
        virtual bool isStringExpression(const std::string& propertyName) const;

        virtual std::string getPropertySymbolizerId(const std::string& propertyName) const;

        virtual void setSymbolizerParameter(const std::shared_ptr<mvt::Symbolizer>& symbolizer, const std::string& name, const std::shared_ptr<const Expression>& expr, bool stringExpr) const;

        std::shared_ptr<mvt::Logger> _logger;

    private:
        struct ValueBuilder;
        
        static const std::vector<std::string> _symbolizerList;
        static const std::unordered_set<std::string> _symbolizerNonStringProperties;
        static const std::unordered_map<std::string, std::string> _symbolizerPropertyMap;
    };
} }

#endif
