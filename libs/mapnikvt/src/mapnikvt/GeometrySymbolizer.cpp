#include "GeometrySymbolizer.h"

namespace carto { namespace mvt {
    void GeometrySymbolizer::bindParameter(const std::string& name, const std::string& value) {
        if (name == "geometry-transform") {
            bind(&_geometryTransform, parseStringExpression(value), &GeometrySymbolizer::convertOptionalTransform);
        }
        else if (name == "comp-op") {
            bind(&_compOp, parseStringExpression(value));
        }
        else {
            Symbolizer::bindParameter(name, value);
        }
    }
} }
