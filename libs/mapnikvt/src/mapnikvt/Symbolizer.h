/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_SYMBOLIZER_H_
#define _CARTO_MAPNIKVT_SYMBOLIZER_H_

#include "FeatureCollection.h"
#include "Expression.h"
#include "ExpressionContext.h"
#include "ExpressionBinder.h"
#include "ParserUtils.h"
#include "SymbolizerContext.h"
#include "Logger.h"
#include "vt/Color.h"
#include "vt/TileLayerBuilder.h"

#include <memory>
#include <mutex>

#include <cglib/mat.h>

namespace carto { namespace mvt {
    class Symbolizer {
    public:
        virtual ~Symbolizer() = default;

        void setParameter(const std::string& name, const std::string& value);
        const std::map<std::string, std::string>& getParameterMap() const;
        const std::vector<std::shared_ptr<const Expression>>& getParameterExpressions() const;

        virtual void build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) = 0;

    protected:
        explicit Symbolizer(std::shared_ptr<Logger> logger) : _logger(std::move(logger)) { }

        vt::CompOp convertCompOp(const std::string& compOp) const;
        vt::LabelOrientation convertLabelPlacement(const std::string& orientation) const;

        vt::Color convertColor(const Value& val) const;
        cglib::mat3x3<float> convertTransform(const Value& val) const;
        boost::optional<cglib::mat3x3<float>> convertOptionalTransform(const Value& val) const;

        virtual void bindParameter(const std::string& name, const std::string& value);

        static long long getTextId(long long id, std::size_t hash);
        static long long getShieldId(long long id, std::size_t hash);
        static long long getBitmapId(long long id, const std::string& file);
        static long long getMultiTextId(long long id, std::size_t hash);
        static long long getMultiShieldId(long long id, std::size_t hash);
        static long long getMultiBitmapId(long long id, const std::string& file);

        template <typename V>
        void bind(V* field, const std::shared_ptr<const Expression>& expr) {
            _binder.bind(field, expr);
            if (!std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                _parameterExprs.push_back(expr);
            }
        }

        template <typename V>
        void bind(V* field, const std::shared_ptr<const Expression>& expr, V (*convertFn)(const Value& val)) {
            _binder.bind(field, expr, convertFn);
            if (!std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                _parameterExprs.push_back(expr);
            }
        }

        template <typename V>
        void bind(V* field, const std::shared_ptr<const Expression>& expr, V (Symbolizer::*memberConvertFn)(const Value& val) const) {
            _binder.bind(field, expr, memberConvertFn, this);
            if (!std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                _parameterExprs.push_back(expr);
            }
        }

        void bind(std::shared_ptr<const vt::FloatFunction>* field, const std::shared_ptr<const Expression>& expr) {
            _functionBinder.bind(field, expr);
            if (!std::dynamic_pointer_cast<const ConstExpression>(expr)) {
                _parameterExprs.push_back(expr);
            }
        }

        void updateBindings(const FeatureExpressionContext& exprContext) {
            _binder.evaluate(exprContext);
            _functionBinder.evaluate(exprContext);
        }

        mutable std::mutex _mutex; // guards internal state as bindings may update it

        std::shared_ptr<Logger> _logger;

    private:
        ExpressionBinder<Symbolizer> _binder;
        ExpressionFunctionBinder _functionBinder;
        std::map<std::string, std::string> _parameterMap;
        std::vector<std::shared_ptr<const Expression>> _parameterExprs;
    };
} }

#endif
