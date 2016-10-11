#include "Symbolizer.h"
#include "FeatureCollection.h"
#include "Expression.h"
#include "ExpressionBinder.h"
#include "Transform.h"
#include "ParserUtils.h"
#include "SymbolizerContext.h"

#include <atomic>

namespace carto { namespace mvt {
    void Symbolizer::setParameter(const std::string& name, const std::string& value) {
        _parameterMap[name] = value;
        bindParameter(name, value);
    }
    
    const std::map<std::string, std::string>& Symbolizer::getParameterMap() const {
        return _parameterMap;
    }

    const std::vector<std::shared_ptr<const Expression>>& Symbolizer::getParameterExpressions() const {
        return _parameterExprs;
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

    vt::PointOrientation Symbolizer::convertLabelToPointOrientation(vt::LabelOrientation orientation) const {
        switch (orientation) {
        case vt::LabelOrientation::BILLBOARD_2D:
            return vt::PointOrientation::BILLBOARD_2D;
        case vt::LabelOrientation::BILLBOARD_3D:
            return vt::PointOrientation::BILLBOARD_3D;
        default: // LabelOrientation::POINT, LabelOrientation::POINT_FLIPPING, LabelOrientation::LINE
            return vt::PointOrientation::POINT;
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

    long long Symbolizer::generateId() {
        static std::atomic<int> counter = ATOMIC_VAR_INIT(0);
        return 0x4000000LL | counter++;
    }

    long long Symbolizer::getTextId(long long id, std::size_t hash) {
        if (id == 0) {
            id = generateId(); // if id is missing, generate new id
        }
        return (id * 3 + 0) | (static_cast<long long>(hash & 0x7fffffff) << 32);
    }

    long long Symbolizer::getShieldId(long long id, std::size_t hash) {
        if (id == 0) {
            id = generateId(); // if id is missing, generate new id
        }
        return (id * 3 + 1) | (static_cast<long long>(hash & 0x7fffffff) << 32);
    }

    long long Symbolizer::getBitmapId(long long id, const std::string& file) {
        if (id == 0) {
            id = generateId(); // if id is missing, generate new id
        }
        std::size_t hash = std::hash<std::string>()(file);
        return (id * 3 + 2) | (static_cast<long long>(hash & 0x7fffffff) << 32);
    }
} }
