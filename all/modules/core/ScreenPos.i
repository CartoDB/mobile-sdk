#ifndef _SCREENPOS_I
#define _SCREENPOS_I

%module ScreenPos

%{
#include "core/ScreenPos.h"
%}

%include <std_string.i>
%include <std_vector.i>
%include <cartoswig.i>

!value_type(carto::ScreenPos, core.ScreenPos)
!value_type(std::vector<carto::ScreenPos>, core.ScreenPosVector)

%attribute(carto::ScreenPos, float, X, getX)
%attribute(carto::ScreenPos, float, Y, getY)
%rename(get) carto::ScreenPos::operator[] const;
%ignore carto::ScreenPos::operator[];
%ignore carto::ScreenPos::setX;
%ignore carto::ScreenPos::setY;
%ignore carto::ScreenPos::setCoords;
!custom_equals(carto::ScreenPos);
!custom_tostring(carto::ScreenPos);

%include "core/ScreenPos.h"

!value_template(std::vector<carto::ScreenPos>, core.ScreenPosVector)

#endif
