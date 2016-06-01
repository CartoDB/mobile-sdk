#ifndef _MAPBOUNDS_I
#define _MAPBOUNDS_I

%module MapBounds

!proxy_imports(carto::MapBounds, core.MapPos, core.MapVec)

%{
#include "core/MapBounds.h"
%}

%include <std_string.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "core/MapVec.i"

!value_type(carto::MapBounds, core.MapBounds)

%attributeval(carto::MapBounds, carto::MapPos, Max, getMax)
%attributeval(carto::MapBounds, carto::MapPos, Center, getCenter)
%attributeval(carto::MapBounds, carto::MapPos, Min, getMin)
%attributeval(carto::MapBounds, carto::MapVec, Delta, getDelta)
!objc_rename(containsPos) carto::MapBounds::contains(const MapPos&) const;
!objc_rename(containsBounds) carto::MapBounds::contains(const MapBounds&) const;
%ignore carto::MapBounds::setBounds;
%ignore carto::MapBounds::setMin;
%ignore carto::MapBounds::setMax;
%ignore carto::MapBounds::expandToContain;
!custom_equals(carto::MapBounds);
!custom_tostring(carto::MapBounds);

%include "core/MapBounds.h"

#endif
