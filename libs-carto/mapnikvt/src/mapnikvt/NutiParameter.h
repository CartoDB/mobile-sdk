/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_NUTIPARAMETER_H_
#define _CARTO_MAPNIKVT_NUTIPARAMETER_H_

#include "Value.h"

#include <memory>
#include <string>
#include <map>

namespace carto { namespace mvt {
    class NutiParameter {
    public:
        explicit NutiParameter(std::string name, Value defaultValue, std::map<std::string, Value> enumMap) : _name(std::move(name)), _defaultValue(std::move(defaultValue)), _enumMap(std::move(enumMap)) { }

        const std::string& getName() const { return _name; }
        const Value& getDefaultValue() const { return _defaultValue; }
        const std::map<std::string, Value>& getEnumMap() const { return _enumMap; }

    private:
        std::string _name;
        Value _defaultValue;
        std::map<std::string, Value> _enumMap;
    };
} }

#endif
