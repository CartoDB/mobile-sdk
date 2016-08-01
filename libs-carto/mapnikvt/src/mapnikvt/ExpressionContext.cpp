#include "ExpressionContext.h"
#include "Expression.h"
#include "FeatureData.h"
#include "ScaleUtils.h"
#include "ValueConverter.h"

#include <map>
#include <memory>

#include <boost/lexical_cast.hpp>

namespace carto { namespace mvt {
    FeatureExpressionContext::FeatureExpressionContext() {
        _scaleDenom = zoom2ScaleDenominator(static_cast<float>(_zoom));
    }
    
    void FeatureExpressionContext::setZoom(int zoom) {
        _zoom = zoom;
        _scaleDenom = zoom2ScaleDenominator(static_cast<float>(_zoom));
    }

    Value FeatureExpressionContext::getVariable(const std::string& name) const {
        if (_featureData) {
            Value value;
            if (_featureData->getVariable(name, value)) {
                return value;
            }
            if (name.compare("mapnik::geometry_type") == 0) {
                return Value(static_cast<long long>(_featureData->getGeometryType()));
            }
        }
        if (name.compare(0, 6, "nuti::") == 0) {
            auto it = _nutiParameterValueMap.find(name.substr(6));
            if (it != _nutiParameterValueMap.end()) {
                return it->second;
            }
        }
        return Value();
    }

    ViewExpressionContext::ViewExpressionContext() {
    }

    void ViewExpressionContext::setZoom(float zoom) {
        _zoom = zoom;
    }

    Value ViewExpressionContext::getVariable(const std::string& name) const {
        if (name == "view::zoom") {
            return Value(_zoom);
        }
        return Value();
    }

    bool ViewExpressionContext::isViewVariable(const std::string& name) {
        return name.compare(0, 6, "view::") == 0;
    }
} }
