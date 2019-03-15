#ifndef _EPSG4326_I
#define _EPSG4326_I

%module EPSG4326

!proxy_imports(carto::EPSG4326, core.MapBounds, core.MapPos, projections.Projection)

%{
#include "projections/EPSG4326.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "projections/Projection.i"

!polymorphic_shared_ptr(carto::EPSG4326, projections.EPSG4326)

%include "projections/EPSG4326.h"

#endif
