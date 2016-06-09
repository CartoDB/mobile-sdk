#ifndef _MAPTILE_I
#define _MAPTILE_I

#pragma SWIG nowarn=317

%module MapTile

%{
#include "core/MapTile.h"
%}

%include <std_string.i>
%include <cartoswig.i>

!value_type(carto::MapTile, core.MapTile)

%attribute(carto::MapTile, int, X, getX)
%attribute(carto::MapTile, int, Y, getY)
%attribute(carto::MapTile, int, Zoom, getZoom)
%attribute(carto::MapTile, int, FrameNr, getFrameNr)
%attribute(carto::MapTile, long long, TileId, getTileId)
%ignore carto::MapTile::getParent;
%ignore carto::MapTile::getChild;
%ignore carto::MapTile::getFlipped;
!custom_equals(carto::MapTile);
!custom_tostring(carto::MapTile);

%include "core/MapTile.h"

#endif
