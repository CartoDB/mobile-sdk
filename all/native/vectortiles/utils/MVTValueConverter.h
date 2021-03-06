/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MVTVALUECONVERTER_H_
#define _CARTO_MVTVALUECONVERTER_H_

#include "core/Variant.h"

#include <mapnikvt/Value.h>

namespace carto {

    struct MVTValueConverter {
        Variant operator() (std::monostate) const { return Variant(); }
        template <typename T> Variant operator() (T val) const { return Variant(val); }
    };

}

#endif
