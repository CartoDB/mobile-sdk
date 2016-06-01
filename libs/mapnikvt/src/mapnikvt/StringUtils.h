/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_STRINGUTILS_H_
#define _CARTO_MAPNIKVT_STRINGUTILS_H_

#include <string>

namespace carto { namespace mvt {
    std::string toUpper(const std::string& str);
    std::string toLower(const std::string& str);
    std::string capitalize(const std::string& str);
    bool regexMatch(const std::string& str, const std::string& re);
    std::string regexReplace(const std::string& str, const std::string& re, const std::string& replacement);
} }

#endif
