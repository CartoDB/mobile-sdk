#ifndef _MAPPOS_I
#define _MAPPOS_I

#pragma SWIG nowarn=317

%module MapPos

!proxy_imports(carto::MapPos, core.MapVec)

%{
#include "core/MapPos.h"
#include "core/MapVec.h"
%}

%include <std_string.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/MapVec.i" 

!value_type(carto::MapPos, core.MapPos)
!value_type(std::vector<carto::MapPos>, core.MapPosVector)
!value_type(std::vector<std::vector<carto::MapPos> >, core.MapPosVectorVector)

%attribute(carto::MapPos, double, Z, getZ)
%attribute(carto::MapPos, double, X, getX)
%attribute(carto::MapPos, double, Y, getY)
%rename(add) carto::MapPos::operator+;
%rename(subVec) carto::MapPos::operator-(const MapVec &) const;
%rename(subPos) carto::MapPos::operator-(const MapPos &) const;
%rename(get) carto::MapPos::operator[] const;
%ignore carto::MapPos::operator[];
%ignore carto::MapPos::setX;
%ignore carto::MapPos::setY;
%ignore carto::MapPos::setZ;
%ignore carto::MapPos::setCoords;
%ignore carto::MapPos::operator!=;
%ignore carto::MapPos::operator+=;
%ignore carto::MapPos::operator-=;
%ignore carto::MapPos::rotate2D;
!custom_equals(carto::MapPos);
!custom_tostring(carto::MapPos);

%include "core/MapPos.h"

!value_template(std::vector<carto::MapPos>, core.MapPosVector)
!value_template(std::vector<std::vector<carto::MapPos> >, core.MapPosVectorVector)

#endif
