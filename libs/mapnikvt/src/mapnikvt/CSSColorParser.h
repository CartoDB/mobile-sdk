/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_CSSCOLORPARSER_H_
#define _CARTO_MAPNIKVT_CSSCOLORPARSER_H_

#include <string>

namespace carto { namespace mvt {
    bool parseCSSColor(std::string name, unsigned int& value);
} }

#endif
