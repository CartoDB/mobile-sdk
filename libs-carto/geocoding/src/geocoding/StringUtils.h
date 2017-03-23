/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_STRINGUTILS_H_
#define _CARTO_GEOCODING_STRINGUTILS_H_

#include <string>

namespace carto { namespace geocoding {
    using unichar_t = std::uint32_t;
    using unistring = std::basic_string<unichar_t>;

    unistring toUniString(const std::string& utf8str);
    std::string toUtf8String(const unistring& unistr);
    
    unistring toLower(const unistring& str);
    unistring toUpper(const unistring& str);
} }

#endif
