#ifndef _MAPRANGE_I
#define _MAPRANGE_I

%module MapRange

%{
#include "core/MapRange.h"
%}

%include <std_string.i>
%include <cartoswig.i>

!value_type(carto::MapRange, core.MapRange)

%attribute(carto::MapRange, float, Max, getMax)
%attribute(carto::MapRange, float, Min, getMin)
%attribute(carto::MapRange, float, Length, length)
%ignore carto::MapRange::setRange;
%ignore carto::MapRange::setMin;
%ignore carto::MapRange::setMax;
!custom_equals(carto::MapRange);
!custom_tostring(carto::MapRange);

%include "core/MapRange.h"

#endif
