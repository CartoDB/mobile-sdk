#include "CartoCSSMapnikTranslator.h"
#include "Expression.h"
#include "Predicate.h"
#include "CartoCSSParser.h"
#include "CartoCSSCompiler.h"
#include "Mapnikvt/Expression.h"
#include "Mapnikvt/ExpressionOperator.h"
#include "Mapnikvt/Predicate.h"
#include "Mapnikvt/PredicateOperator.h"
#include "Mapnikvt/Filter.h"
#include "Mapnikvt/Rule.h"
#include "Mapnikvt/Style.h"
#include "Mapnikvt/Layer.h"
#include "Mapnikvt/PointSymbolizer.h"
#include "Mapnikvt/LineSymbolizer.h"
#include "Mapnikvt/LinePatternSymbolizer.h"
#include "Mapnikvt/PolygonSymbolizer.h"
#include "Mapnikvt/PolygonPatternSymbolizer.h"
#include "Mapnikvt/TextSymbolizer.h"
#include "Mapnikvt/MarkersSymbolizer.h"
#include "Mapnikvt/ShieldSymbolizer.h"
#include "Mapnikvt/BuildingSymbolizer.h"
#include "Mapnikvt/GeneratorUtils.h"

namespace carto { namespace css {
    struct CartoCSSMapnikTranslator::ValueBuilder : boost::static_visitor<mvt::Value> {
        mvt::Value operator() (boost::blank) const { return mvt::Value(); }
        mvt::Value operator() (bool val) const { return mvt::Value(val); }
        mvt::Value operator() (long long val) const { return mvt::Value(val); }
        mvt::Value operator() (double val) const { return mvt::Value(val); }
        mvt::Value operator() (const Color& val) const { std::stringstream ss; ss << val; return mvt::Value(ss.str()); }
        mvt::Value operator() (const std::string& val) const { return mvt::Value(val); }
        mvt::Value operator() (const std::vector<Value>& val) const { return mvt::Value(boost::lexical_cast<std::string>(val)); }
    };

    std::shared_ptr<mvt::Rule> CartoCSSMapnikTranslator::buildRule(const CartoCSSCompiler::PropertySet& propertySet, const std::shared_ptr<mvt::Map>& map, int zoom) const {
        std::shared_ptr<mvt::Predicate> mapnikFilterPred;
        for (const std::shared_ptr<const Predicate>& pred : propertySet.filters) {
            std::shared_ptr<mvt::Predicate> mapnikPred = buildPredicate(pred);
            if (mapnikPred) {
                if (mapnikFilterPred) {
                    mapnikFilterPred = std::make_shared<mvt::AndPredicate>(mapnikFilterPred, mapnikPred);
                }
                else {
                    mapnikFilterPred = mapnikPred;
                }
            }
        }
        auto mapnikFilter = std::make_shared<mvt::Filter>(mvt::Filter::Type::FILTER, mapnikFilterPred);
        std::list<std::pair<std::string, std::list<CartoCSSCompiler::Property>>> propertyLists;
        std::vector<std::pair<std::string, CartoCSSCompiler::Property>> properties(propertySet.properties.begin(), propertySet.properties.end());
        std::sort(properties.begin(), properties.end(), [](const std::pair<std::string, CartoCSSCompiler::Property>& prop1, const std::pair<std::string, CartoCSSCompiler::Property>& prop2) {
            return std::get<3>(prop1.second.specificity) < std::get<3>(prop2.second.specificity);
        });
        for (auto it = properties.begin(); it != properties.end(); it++) {
            std::string symbolizerId = getPropertySymbolizerId(it->first);
            if (symbolizerId.empty()) { // layer-level property? (comp-op, opacity)
                continue;
            }
            auto it2 = std::find_if(propertyLists.begin(), propertyLists.end(), [&](const std::pair<std::string, std::list<CartoCSSCompiler::Property>>& propertyListElement) { return propertyListElement.first == symbolizerId; });
            if (it2 == propertyLists.end()) {
                it2 = propertyLists.insert(it2, std::make_pair(symbolizerId, std::list<CartoCSSCompiler::Property>()));
            }
            it2->second.push_back(it->second);
        }
        std::vector<std::shared_ptr<mvt::Symbolizer>> mapnikSymbolizers;
        for (const std::pair<std::string, std::list<CartoCSSCompiler::Property>>& propertyListElement : propertyLists) {
            std::string symbolizerId = propertyListElement.first;
            std::string symbolizerType = symbolizerId.substr(symbolizerId.rfind('/') + 1);
            std::shared_ptr<mvt::Symbolizer> mapnikSymbolizer = buildSymbolizer(symbolizerType, propertyListElement.second, map, zoom);
            if (mapnikSymbolizer) {
                mapnikSymbolizers.push_back(mapnikSymbolizer);
            }
        }
        return std::make_shared<mvt::Rule>("auto", zoom, zoom + 1, mapnikFilter, mapnikSymbolizers);
    }

    std::shared_ptr<mvt::Symbolizer> CartoCSSMapnikTranslator::buildSymbolizer(const std::string& symbolizerType, const std::list<CartoCSSCompiler::Property>& properties, const std::shared_ptr<mvt::Map>& map, int zoom) const {
        std::shared_ptr<mvt::Symbolizer> mapnikSymbolizer;
        if (symbolizerType == "line") {
            mapnikSymbolizer = std::make_shared<mvt::LineSymbolizer>(_logger);
        }
        else if (symbolizerType == "line-pattern") {
            mapnikSymbolizer = std::make_shared<mvt::LinePatternSymbolizer>(_logger);
        }
        else if (symbolizerType == "polygon") {
            mapnikSymbolizer = std::make_shared<mvt::PolygonSymbolizer>(_logger);
        }
        else if (symbolizerType == "polygon-pattern") {
            mapnikSymbolizer = std::make_shared<mvt::PolygonPatternSymbolizer>(_logger);
        }
        else if (symbolizerType == "marker") {
            mapnikSymbolizer = std::make_shared<mvt::MarkersSymbolizer>(_logger);
        }
        else if (symbolizerType == "text" || symbolizerType == "shield") {
            std::shared_ptr<mvt::Expression> textExpr;
            for (const CartoCSSCompiler::Property& prop : properties) {
                if (prop.field == symbolizerType + "-name") {
                    try {
                        if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(prop.expression)) {
                            std::string value = boost::lexical_cast<std::string>(constExpr->getValue());
                            if (value.empty()) {
                                return std::shared_ptr<mvt::Symbolizer>();
                            }
                            textExpr = mvt::parseStringExpression(value);
                        }
                        else {
                            textExpr = mvt::parseExpression(buildEscapedExpressionString(prop.expression));
                        }
                    }
                    catch (const std::runtime_error& ex) {
                        _logger->write(mvt::Logger::Severity::ERROR, ex.what());
                    }
                }
                else if (prop.field == symbolizerType + "-face-name") {
                    if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(prop.expression)) {
                        Value value = constExpr->getValue();
                        if (auto faceNameValues = boost::get<std::vector<Value>>(&value)) {
                            std::string fontSetName = boost::lexical_cast<std::string>(value);
                            if (!map->getFontSet(fontSetName)) {
                                std::vector<std::string> faceNames;
                                for (const Value& faceName : *faceNameValues) {
                                    faceNames.push_back(boost::lexical_cast<std::string>(faceName));
                                }
                                auto fontSet = std::make_shared<mvt::FontSet>(fontSetName, faceNames);
                                map->addFontSet(fontSet);
                            }
                        }
                    }
                }
            }
            if (textExpr) {
                if (symbolizerType == "text") {
                    mapnikSymbolizer = std::make_shared<mvt::TextSymbolizer>(map->getFontSets(), _logger);
                }
                else {
                    mapnikSymbolizer = std::make_shared<mvt::ShieldSymbolizer>(map->getFontSets(), _logger);
                }
                std::static_pointer_cast<mvt::TextSymbolizer>(mapnikSymbolizer)->setTextExpression(textExpr);
            }
            else {
                _logger->write(mvt::Logger::Severity::ERROR, "Unsupported text expression type");
                return std::shared_ptr<mvt::Symbolizer>();
            }
            for (const CartoCSSCompiler::Property& prop : properties) {
                if (prop.field == symbolizerType + "-face-name") {
                    if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(prop.expression)) {
                        Value value = constExpr->getValue();
                        if (boost::get<std::vector<Value>>(&value)) {
                            mapnikSymbolizer->setParameter("fontset-name", boost::lexical_cast<std::string>(value));
                        }
                        else {
                            mapnikSymbolizer->setParameter("face-name", boost::lexical_cast<std::string>(value));
                        }
                    }
                }
            }
        }
        else if (symbolizerType == "building") {
            mapnikSymbolizer = std::make_shared<mvt::BuildingSymbolizer>(_logger);
        }
        else {
            _logger->write(mvt::Logger::Severity::ERROR, "Unsupported symbolizer type: " + symbolizerType);
            return std::shared_ptr<mvt::Symbolizer>();
        }

        for (const CartoCSSCompiler::Property& prop : properties) {
            std::string propertyId = prop.field.substr(prop.field.rfind('/') + 1);
            auto it = _symbolizerPropertyMap.find(propertyId);
            if (it == _symbolizerPropertyMap.end()) {
                _logger->write(mvt::Logger::Severity::WARNING, "Unsupported symbolizer property: " + propertyId);
                continue;
            }
            if (!it->second.empty()) {
                try {
                    mapnikSymbolizer->setParameter(it->second, buildExpressionString(prop.expression));
                }
                catch (const std::runtime_error& ex) {
                    _logger->write(mvt::Logger::Severity::ERROR, ex.what());
                }
            }
        }

        return mapnikSymbolizer;
    }

    std::string CartoCSSMapnikTranslator::buildExpressionString(const std::shared_ptr<const Expression>& expr) const {
        if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(expr)) {
            return boost::lexical_cast<std::string>(buildValue(constExpr->getValue()));
        }
        return buildEscapedExpressionString(expr);
    }

    std::string CartoCSSMapnikTranslator::buildEscapedExpressionString(const std::shared_ptr<const Expression>& expr) const {
        if (auto constExpr = std::dynamic_pointer_cast<const ConstExpression>(expr)) {
            return mvt::generateValueString(buildValue(constExpr->getValue()));
        }
        else if (auto fieldVarExpr = std::dynamic_pointer_cast<const FieldOrVarExpression>(expr)) {
            if (!fieldVarExpr->isField()) {
                throw TranslatorException("FieldOrVarExpression: expecting field, not variable (@" + fieldVarExpr->getFieldOrVar() + ")");
            }
            return "[" + fieldVarExpr->getFieldOrVar() + "]";
        }
        else if (auto unaryExpr = std::dynamic_pointer_cast<const UnaryExpression>(expr)) {
            std::string exprStr = buildEscapedExpressionString(unaryExpr->getExpression());
            std::string op;
            switch (unaryExpr->getOp()) {
            case UnaryExpression::Op::NEG:
                op = "-";
                break;
            default:
                throw TranslatorException("Unsupported unary operator type");
            }
            return op + "(" + exprStr + ")";
        }
        else if (auto binaryExpr = std::dynamic_pointer_cast<const BinaryExpression>(expr)) {
            std::string expr1Str = buildEscapedExpressionString(binaryExpr->getExpression1());
            std::string expr2Str = buildEscapedExpressionString(binaryExpr->getExpression2());
            std::string op;
            switch (binaryExpr->getOp()) {
            case BinaryExpression::Op::ADD:
                op = "+";
                break;
            case BinaryExpression::Op::SUB:
                op = "-";
                break;
            case BinaryExpression::Op::MUL:
                op = "*";
                break;
            case BinaryExpression::Op::DIV:
                op = "/";
                break;
            default:
                throw TranslatorException("Unsupported binary operator type");
            }
            return "(" + expr1Str + ")" + op + "(" + expr2Str + ")";
        }
        else if (auto funcExpr = std::dynamic_pointer_cast<const FunctionExpression>(expr)) {
            std::string exprStr = funcExpr->getFunc();
            exprStr += "(";
            for (std::size_t i = 0; i < funcExpr->getArgs().size(); i++) {
                if (i > 0) {
                    exprStr += ",";
                }
                exprStr += buildEscapedExpressionString(funcExpr->getArgs()[i]);
            }
            exprStr += ")";
            return exprStr;
        }
        throw TranslatorException("Unsupported expression type");
    }

    std::shared_ptr<mvt::Predicate> CartoCSSMapnikTranslator::buildPredicate(const std::shared_ptr<const Predicate>& pred) const {
        if (std::dynamic_pointer_cast<const MapPredicate>(pred)) {
            return std::shared_ptr<mvt::Predicate>();
        }
        else if (std::dynamic_pointer_cast<const LayerPredicate>(pred)) {
            return std::shared_ptr<mvt::Predicate>();
        }
        else if (auto clsPred = std::dynamic_pointer_cast<const ClassPredicate>(pred)) {
            if (std::shared_ptr<mvt::ComparisonPredicate::Operator> op = buildOperator(OpPredicate::Op::EQ)) {
                mvt::Value val = buildValue(clsPred->getClass());
                return std::make_shared<mvt::ComparisonPredicate>(op, std::make_shared<mvt::VariableExpression>("class"), std::make_shared<mvt::ConstExpression>(val));
            }
        }
        else if (auto opPred = std::dynamic_pointer_cast<const OpPredicate>(pred)) {
            if (!opPred->isField()) {
                throw TranslatorException("OpPredicate: expecting field, not variable (@" + opPred->getFieldOrVar() + ")");
            }
            if (std::shared_ptr<mvt::ComparisonPredicate::Operator> op = buildOperator(opPred->getOp())) {
                std::string var = opPred->getFieldOrVar();
                mvt::Value val = buildValue(opPred->getRefValue());
                return std::make_shared<mvt::ComparisonPredicate>(op, std::make_shared<mvt::VariableExpression>(var), std::make_shared<mvt::ConstExpression>(val));
            }
        }
        throw TranslatorException("Unsupported predicate type");
    }

    std::shared_ptr<mvt::ComparisonPredicate::Operator> CartoCSSMapnikTranslator::buildOperator(OpPredicate::Op op) const {
        switch (op) {
        case OpPredicate::Op::EQ:
            return std::make_shared<mvt::EQOperator>();
        case OpPredicate::Op::NEQ:
            return std::make_shared<mvt::NEQOperator>();
        case OpPredicate::Op::LT:
            return std::make_shared<mvt::LTOperator>();
        case OpPredicate::Op::LTE:
            return std::make_shared<mvt::LTEOperator>();
        case OpPredicate::Op::GT:
            return std::make_shared<mvt::GTOperator>();
        case OpPredicate::Op::GTE:
            return std::make_shared<mvt::GTEOperator>();
        case OpPredicate::Op::MATCH:
            return std::make_shared<mvt::MatchOperator>();
        }
        throw TranslatorException("Unsupported predicate operator");
    }

    mvt::Value CartoCSSMapnikTranslator::buildValue(const Value& val) const {
        return boost::apply_visitor(ValueBuilder(), val);
    }

    const std::vector<std::string> CartoCSSMapnikTranslator::_symbolizerList = {
        "line-pattern",
        "line",
        "polygon-pattern",
        "polygon",
        "point",
        "text",
        "marker",
        "shield",
        "building",
    };

    std::string CartoCSSMapnikTranslator::getPropertySymbolizerId(const std::string& propertyName) const {
        std::string::size_type symbolizerTypePos = propertyName.rfind('/') + 1;
        for (const std::string& symbolizerType : _symbolizerList) {
            if (propertyName.substr(symbolizerTypePos, symbolizerType.size()) == symbolizerType) {
                return propertyName.substr(0, symbolizerTypePos + symbolizerType.size());
            }
        }
        return std::string();
    }

    const std::map<std::string, std::string> CartoCSSMapnikTranslator::_symbolizerPropertyMap = {
        { "line-color", "stroke" },
        { "line-opacity", "stroke-opacity" },
        { "line-width", "stroke-width" },
        { "line-dasharray", "stroke-dasharray" },
        { "line-join", "stroke-linejoin" },
        { "line-cap", "stroke-linecap" },
        { "line-geometry-transform", "geometry-transform" },
        { "line-comp-op", "comp-op" },

        { "line-pattern-file", "file" },
        { "line-pattern-fill", "fill" },
        { "line-pattern-opacity", "opacity" },
        { "line-pattern-geometry-transform", "geometry-transform" },
        { "line-pattern-comp-op", "comp-op" },

        { "polygon-fill", "fill" },
        { "polygon-opacity", "fill-opacity" },
        { "polygon-geometry-transform", "geometry-transform" },
        { "polygon-comp-op", "comp-op" },

        { "polygon-pattern-file", "file" },
        { "polygon-pattern-fill", "fill" },
        { "polygon-pattern-opacity", "opacity" },
        { "polygon-pattern-geometry-transform", "geometry-transform" },
        { "polygon-pattern-op", "comp-op" },

        { "point-file", "file" },
        { "point-opacity", "opacity" },
        { "point-allow-overlap", "allow-overlap" },
        { "point-ignore-placement", "ignore-placement" },
        { "point-transform", "transform" },

        { "text-name", "" },
        { "text-face-name", "" },
        { "text-placement", "placement" },
        { "text-size", "size" },
        { "text-spacing", "spacing" },
        { "text-fill", "fill" },
        { "text-opacity", "opacity" },
        { "text-halo-fill", "halo-fill" },
        { "text-halo-opacity", "halo-opacity" },
        { "text-halo-radius", "halo-radius" },
        { "text-halo-rasterizer", "halo-rasterizer" },
        { "text-allow-overlap", "allow-overlap" },
        { "text-min-distance", "minimum-distance" },
        { "text-transform", "text-transform" },
        { "text-orientation", "orientation" },
        { "text-dx", "dx" },
        { "text-dy", "dy" },
        { "text-avoid-edges", "avoid-edges" },
        { "text-wrap-width", "wrap-width" },
        { "text-wrap-before", "wrap-before" },
        { "text-character-spacing", "character-spacing" },
        { "text-line-spacing", "line-spacing" },
        { "text-horizontal-alignment", "horizontal-alignment" },
        { "text-vertical-alignment", "vertical-alignment" },

        { "shield-name", "" },
        { "shield-face-name", "" },
        { "shield-file", "file" },
        { "shield-dx", "shield-dx" },
        { "shield-dy", "shield-dy" },
        { "shield-unlock-image", "unlock-image" },
        { "shield-placement", "placement" },
        { "shield-size", "size" },
        { "shield-spacing", "spacing" },
        { "shield-fill", "fill" },
        { "shield-opacity", "opacity" },
        { "shield-halo-fill", "halo-fill" },
        { "shield-halo-opacity", "halo-opacity" },
        { "shield-halo-radius", "halo-radius" },
        { "shield-halo-rasterizer", "halo-rasterizer" },
        { "shield-allow-overlap", "allow-overlap" },
        { "shield-min-distance", "minimum-distance" },
        { "shield-transform", "text-transform" },
        { "shield-orientation", "orientation" },
        { "shield-text-dx", "dx" },
        { "shield-text-dy", "dy" },
        { "shield-avoid-edges", "avoid-edges" },
        { "shield-wrap-width", "wrap-width" },
        { "shield-wrap-before", "wrap-before" },
        { "shield-character-spacing", "character-spacing" },
        { "shield-line-spacing", "line-spacing" },
        { "shield-horizontal-alignment", "horizontal-alignment" },
        { "shield-vertical-alignment", "vertical-alignment" },

        { "marker-file", "file" },
        { "marker-placement", "placement" },
        { "marker-type", "marker-type" },
        { "marker-opacity", "opacity" },
        { "marker-fill", "fill" },
        { "marker-fill-opacity", "fill-opacity" },
        { "marker-width", "width" },
        { "marker-height", "height" },
        { "marker-line-color", "stroke" },
        { "marker-line-opacity", "stroke-opacity" },
        { "marker-line-width", "stroke-width" },
        { "marker-spacing", "spacing" },
        { "marker-allow-overlap", "allow-overlap" },
        { "marker-ignore-placement", "ignore-placement" },
        { "marker-transform", "transform" },

        { "building-fill", "fill" },
        { "building-fill-opacity", "fill-opacity" },
        { "building-height", "height" },
        { "building-geometry-transform", "geometry-transform" },
    };
} }
