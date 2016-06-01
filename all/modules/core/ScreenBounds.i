#ifndef _SCREENBOUNDS_I
#define _SCREENBOUNDS_I

%module ScreenBounds

!proxy_imports(carto::ScreenBounds, core.ScreenPos)

%{
#include "core/ScreenBounds.h"
%}

%include <std_string.i>
%include <cartoswig.i>

%import "core/ScreenPos.i"

!value_type(carto::ScreenBounds, core.ScreenBounds)

%attributeval(carto::ScreenBounds, carto::ScreenPos, Min, getMin)
%attributeval(carto::ScreenBounds, carto::ScreenPos, Max, getMax)
%attributeval(carto::ScreenBounds, carto::ScreenPos, Center, getCenter)
!objc_rename(containsPos) carto::ScreenBounds::contains(const ScreenPos&) const;
!objc_rename(containsBounds) carto::ScreenBounds::contains(const ScreenBounds&) const;
%ignore carto::ScreenBounds::setBounds;
%ignore carto::ScreenBounds::setMin;
%ignore carto::ScreenBounds::setMax;
%ignore carto::ScreenBounds::expandToContain;
!custom_equals(carto::ScreenBounds);
!custom_tostring(carto::ScreenBounds);

%include "core/ScreenBounds.h"

#endif
