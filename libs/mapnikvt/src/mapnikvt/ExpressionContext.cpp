#include "ExpressionContext.h"
#include "Expression.h"
#include "FeatureData.h"
#include "ScaleUtils.h"
#include "ValueConverter.h"

#include <map>
#include <memory>

#include <boost/lexical_cast.hpp>

namespace carto { namespace mvt {
    ExpressionContext::ExpressionContext() {
        _scaleDenom = zoom2ScaleDenominator(static_cast<float>(_zoom));
    }
    
    void ExpressionContext::setZoom(int zoom) {
        _zoom = zoom;
        _scaleDenom = zoom2ScaleDenominator(static_cast<float>(_zoom));
    }

    Value ExpressionContext::getVariable(const std::string& name) const {
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
            NutiParameterValueMap::const_iterator it = _nutiParameterValueMap.find(name.substr(6));
            if (it != _nutiParameterValueMap.end()) {
                return it->second;
            }
        }
        return Value();
    }
} }
