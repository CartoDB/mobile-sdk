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
#include "Mapnikvt/Map.h"
#include "Mapnikvt/Rule.h"
#include "Mapnikvt/Expression.h"
#include "Mapnikvt/Predicate.h"
#include "Mapnikvt/Logger.h"

#include <memory>
#include <string>
#include <vector>

namespace carto { namespace css {
    class CartoCSSMapnikTranslator {
    public:
        class TranslatorException : public std::runtime_error {
        public:
            explicit TranslatorException(const std::string& msg) : runtime_error(msg) { }
        };

        explicit CartoCSSMapnikTranslator(std::shared_ptr<mvt::Logger> logger) : _logger(std::move(logger)) { }

        virtual std::shared_ptr<mvt::Rule> buildRule(const CartoCSSCompiler::PropertySet& propertySet, const std::shared_ptr<mvt::Map>& map, int zoom) const;

        virtual std::shared_ptr<mvt::Symbolizer> buildSymbolizer(const std::string& symbolizerType, const std::list<CartoCSSCompiler::Property>& properties, const std::shared_ptr<mvt::Map>& map, int zoom) const;

        virtual std::string buildExpressionString(const std::shared_ptr<const Expression>& expr) const;

        virtual std::string buildEscapedExpressionString(const std::shared_ptr<const Expression>& expr) const;

        virtual std::shared_ptr<mvt::Predicate> buildPredicate(const std::shared_ptr<const Predicate>& pred) const;

        virtual std::shared_ptr<mvt::ComparisonPredicate::Operator> buildOperator(OpPredicate::Op op) const;

    protected:
        virtual mvt::Value buildValue(const Value& val) const;

        virtual std::string getPropertySymbolizerId(const std::string& propertyName) const;

        std::shared_ptr<mvt::Logger> _logger;

    private:
        struct ValueBuilder;
        
        static const std::vector<std::string> _symbolizerList;
        static const std::map<std::string, std::string> _symbolizerPropertyMap;
    };
} }

#endif
