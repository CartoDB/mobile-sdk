#ifndef _PROJECTION_I
#define _PROJECTION_I

%module Projection

!proxy_imports(carto::Projection, core.MapBounds, core.MapPos)

%{
#include "projections/Projection.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapBounds.i"
%import "core/MapPos.i"

!polymorphic_shared_ptr(carto::Projection, projections.Projection)

%attributeval(carto::Projection, carto::MapBounds, Bounds, getBounds)
%attributestring(carto::Projection, std::string, Name, getName)
%ignore carto::Projection::toInternalScale;
%ignore carto::Projection::fromInternal;
%ignore carto::Projection::toInternal;
!standard_equals(carto::Projection);

%include "projections/Projection.h"

#endif
