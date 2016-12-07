/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VALUECONVERTER_H_
#define _CARTO_VALUECONVERTER_H_

#include "core/Variant.h"

#include <mapnikvt/Value.h>

namespace carto {

    struct ValueConverter : boost::static_visitor<Variant> {
        Variant operator() (boost::blank) const { return Variant(); }
        template <typename T> Variant operator() (T val) const { return Variant(val); }
    };

}

#endif
