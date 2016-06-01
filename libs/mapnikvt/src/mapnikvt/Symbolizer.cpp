#include "Symbolizer.h"
#include "FeatureCollection.h"
#include "Expression.h"
#include "ExpressionBinder.h"
#include "Transform.h"
#include "ParserUtils.h"
#include "SymbolizerContext.h"

#include <cstdlib>

namespace carto { namespace mvt {
    void Symbolizer::setParameter(const std::string& name, const std::string& value) {
        _parameterMap[name] = value;
        bindParameter(name, value);
    }
    
    const std::map<std::string, std::string>& Symbolizer::getParameterMap() const {
        return _parameterMap;
    }

    const std::vector<std::shared_ptr<Expression>>& Symbolizer::getParameterExpressions() const {
        return _binder.getExpressions();
    }

    vt::CompOp Symbolizer::convertCompOp(const std::string& compOp) const {
        try {
            return parseCompOp(compOp);
        }
        catch (const ParserException& ex) {
            _logger->write(Logger::Severity::ERROR, ex.what() + std::string(": ") + ex.string());
            return vt::CompOp::SRC_OVER;
        }
    }

    vt::LabelOrientation Symbolizer::convertLabelPlacement(const std::string& orientation) const {
        try {
            return parseLabelOrientation(orientation);
        }
        catch (const ParserException& ex) {
            _logger->write(Logger::Severity::ERROR, ex.what() + std::string(": ") + ex.string());
            return vt::LabelOrientation::LINE;
        }
    }

    vt::Color Symbolizer::convertColor(const Value& val) const {
        try {
            return parseColor(boost::lexical_cast<std::string>(val));
        }
        catch (const ParserException& ex) {
            _logger->write(Logger::Severity::ERROR, ex.what() + std::string(": ") + ex.string());
            return vt::Color();
        }
    }

    cglib::mat3x3<float> Symbolizer::convertTransform(const Value& val) const {
        try {
            std::vector<std::shared_ptr<Transform>> transforms = parseTransformList(boost::lexical_cast<std::string>(val));
            cglib::mat3x3<float> matrix = cglib::mat3x3<float>::identity();
            for (const std::shared_ptr<Transform>& transform : transforms) {
                matrix = matrix * transform->getMatrix();
            }
            return matrix;
        }
        catch (const ParserException& ex) {
            _logger->write(Logger::Severity::ERROR, ex.what() + std::string(": ") + ex.string());
            return cglib::mat3x3<float>::identity();
        }
    }

    boost::optional<cglib::mat3x3<float>> Symbolizer::convertOptionalTransform(const Value& val) const {
        return convertTransform(val);
    }

    void Symbolizer::bindParameter(const std::string& name, const std::string& value) {
        _logger->write(Logger::Severity::WARNING, "Unsupported symbolizer parameter: " + name);
    }

    long long Symbolizer::getTextId(long long id, std::size_t hash) {
        return (id * 3 + 0) | (static_cast<long long>(hash & 0x7fffffff) << 32);
    }

    long long Symbolizer::getShieldId(long long id, std::size_t hash) {
        return (id * 3 + 1) | (static_cast<long long>(hash & 0x7fffffff) << 32);
    }

    long long Symbolizer::getBitmapId(long long id, const std::string& file) {
        std::size_t hash = std::hash<std::string>()(file);
        return (id * 3 + 2) | (static_cast<long long>(hash & 0x7fffffff) << 32);
    }

    long long Symbolizer::getMultiTextId(long long id, std::size_t hash) {
        return (id * 3 + 0) | (static_cast<long long>(std::rand()) << 32); // randomize id generation, basically it means we will create a unique id each time
    }
    
    long long Symbolizer::getMultiShieldId(long long id, std::size_t hash) {
        return (id * 3 + 1) | (static_cast<long long>(std::rand()) << 32); // randomize id generation, basically it means we will create a unique id each time
    }

    long long Symbolizer::getMultiBitmapId(long long id, const std::string& file) {
        return (id * 3 + 2) | (static_cast<long long>(std::rand()) << 32); // randomize id generation, basically it means we will create a unique id each time
    }
} }
