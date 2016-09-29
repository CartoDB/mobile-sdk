/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_FEATUREDATA_H_
#define _CARTO_MAPNIKVT_FEATUREDATA_H_

#include "Value.h"

#include <string>
#include <algorithm>
#include <vector>
#include <unordered_set>

namespace carto { namespace mvt {
    class FeatureData final {
    public:
        enum class GeometryType {
            NULL_GEOMETRY = 0, POINT_GEOMETRY = 1, LINE_GEOMETRY = 2, POLYGON_GEOMETRY = 3
        };

        explicit FeatureData(GeometryType geomType, std::vector<std::pair<std::string, Value>> vars) : _geometryType(geomType), _variables(std::move(vars)) { }

        GeometryType getGeometryType() const { return _geometryType; }

        std::unordered_set<std::string> getVariableNames() const {
             std::unordered_set<std::string> names;
             std::transform(_variables.begin(), _variables.end(), std::inserter(names, names.begin()), [](const std::pair<std::string, Value>& var) {
                 return var.first;
             });
             return names;
        }

        bool getVariable(const std::string& name, Value& value) const {
            auto it = std::find_if(_variables.begin(), _variables.end(), [name](const std::pair<std::string, Value>& var) { return var.first == name; });
            if (it == _variables.end()) {
                return false;
            }
            value = it->second;
            return true;
        }

    private:
        GeometryType _geometryType;
        std::vector<std::pair<std::string, Value>> _variables;
    };
} }

#endif
