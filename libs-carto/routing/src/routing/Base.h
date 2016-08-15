/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTING_BASE_H_
#define _CARTO_ROUTING_BASE_H_

#include <cglib/vec.h>
#include <cglib/bbox.h>

namespace carto { namespace routing {
    using WGSPos = cglib::vec2<double>;
    using WGSBounds = cglib::bbox2<double>;
} }

#endif
