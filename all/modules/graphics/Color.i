#ifndef _COLOR_I
#define _COLOR_I

%module Color

%{
#include "graphics/Color.h"
%}

%include <std_string.i>
%include <cartoswig.i>

!value_type(carto::Color, graphics.Color)

%attribute(carto::Color, int, ARGB, getARGB)
%attribute(carto::Color, unsigned char, R, getR)
%attribute(carto::Color, unsigned char, G, getG)
%attribute(carto::Color, unsigned char, B, getB)
%attribute(carto::Color, unsigned char, A, getA)
!custom_equals(carto::Color);
!custom_tostring(carto::Color);

%include "graphics/Color.h"

#endif
