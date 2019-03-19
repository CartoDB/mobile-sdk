#ifndef _TILEUTILS_I
#define _TILEUTILS_I

%module TileUtils

!proxy_imports(carto::TileUtils, core.MapPos, core.MapBounds, core.MapTile, projections.Projection)

%{
#include "utils/TileUtils.h"
%}

%include <std_shared_ptr.i>

%import "core/MapPos.i"
%import "core/MapBounds.i"
%import "core/MapTile.i"
%import "projections/Projection.i"

%include "utils/TileUtils.h"

#endif
