/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_PARAMETER_H_
#define _CARTO_MAPNIKVT_PARAMETER_H_

#include <memory>
#include <string>

namespace carto { namespace mvt {
    class Parameter final {
    public:
        explicit Parameter(std::string name, std::string value) : _name(std::move(name)), _value(std::move(value)) { }

        const std::string& getName() const { return _name; }
        const std::string& getValue() const { return _value; }

    private:
        std::string _name;
        std::string _value;
    };
} }

#endif
