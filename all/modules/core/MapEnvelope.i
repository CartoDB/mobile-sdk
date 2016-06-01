#ifndef _MAPENVELOPE_I
#define _MAPENVELOPE_I

%module MapEnvelope

!proxy_imports(carto::MapEnvelope, core.MapBounds, core.MapPosVector)

%{
#include "core/MapEnvelope.h"
%}

%include <std_string.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "core/MapBounds.i"

!value_type(carto::MapEnvelope, core.MapEnvelope)

%attributeval(carto::MapEnvelope, carto::MapBounds, Bounds, getBounds)
%attributeval(carto::MapEnvelope, std::vector<carto::MapPos>, ConvexHull, getConvexHull)
!custom_equals(carto::MapEnvelope);
!custom_tostring(carto::MapEnvelope);

%include "core/MapEnvelope.h"

#endif
