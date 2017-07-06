#ifndef _FRUSTUM_I
#define _FRUSTUM_I

%module Frustum

!proxy_imports(carto::Frustum, core.MapBounds, core.MapPos)

%{
#include "graphics/Frustum.h"
%}

%include <cartoswig.i>

%import "core/MapBounds.i"
%import "core/MapPos.i"

!value_type(carto::Frustum, graphics.Frustum)

%ignore carto::Frustum::Frustum;
!standard_equals(carto::Frustum);

%include "graphics/Frustum.h"

#endif
