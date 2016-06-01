#ifndef _EPSG3857_I
#define _EPSG3857_I

%module EPSG3857

!proxy_imports(carto::EPSG3857, core.MapBounds, core.MapPos, projections.Projection)

%{
#include "projections/EPSG3857.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "projections/Projection.i"

!polymorphic_shared_ptr(carto::EPSG3857, projections.EPSG3857)

%include "projections/EPSG3857.h"

#endif
