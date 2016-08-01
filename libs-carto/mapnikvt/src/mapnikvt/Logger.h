/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_LOGGER_H_
#define _CARTO_MAPNIKVT_LOGGER_H_

#include <string>

namespace carto { namespace mvt {
    class Logger {
    public:
        enum class Severity {
            INFO, WARNING, ERROR
        };

        virtual ~Logger() = default;

        virtual void write(Severity severity, const std::string& msg) = 0;
    };
} }

#endif
