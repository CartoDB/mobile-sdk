#ifndef _MAPVEC_I
#define _MAPVEC_I

%module MapVec

%{
#include "core/MapVec.h"
%}

%include <std_string.i>
%include <cartoswig.i>

!value_type(carto::MapVec, core.MapVec)

%attribute(carto::MapVec, double, Z, getZ)
%attribute(carto::MapVec, double, X, getX)
%attribute(carto::MapVec, double, Y, getY)
%attribute(carto::MapVec, double, Length, length)
%attributeval(carto::MapVec, carto::MapVec, Normalized, getNormalized)
%rename(add) carto::MapVec::operator+;
%rename(sub) carto::MapVec::operator-;
%rename(mul) carto::MapVec::operator*;
%rename(div) carto::MapVec::operator/;
%rename(get) carto::MapVec::operator[] const;
%ignore carto::MapVec::operator[];
%ignore carto::MapVec::setX;
%ignore carto::MapVec::setY;
%ignore carto::MapVec::setZ;
%ignore carto::MapVec::setCoords;
%ignore carto::MapVec::operator+=;
%ignore carto::MapVec::operator-=;
%ignore carto::MapVec::operator*=;
%ignore carto::MapVec::operator/=;
%ignore carto::MapVec::rotate2D;
%ignore carto::MapVec::normalize;
%ignore carto::MapVec::lengthSqr;
!custom_equals(carto::MapVec);
!custom_tostring(carto::MapVec);

%include "core/MapVec.h"

#endif
