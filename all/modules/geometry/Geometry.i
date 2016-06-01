#ifndef _GEOMETRY_I
#define _GEOMETRY_I

%module Geometry

!proxy_imports(carto::Geometry, core.MapPos, core.MapBounds)

%{
#include "geometry/Geometry.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "core/MapBounds.i"

!polymorphic_shared_ptr(carto::Geometry, geometry.Geometry)
!value_type(std::vector<std::shared_ptr<carto::Geometry> >, geometry.GeometryVector)

%attributeval(carto::Geometry, carto::MapBounds, Bounds, getBounds)
%attributeval(carto::Geometry, carto::MapPos, CenterPos, getCenterPos)
!standard_equals(carto::Geometry);

%include "geometry/Geometry.h"

!value_template(std::vector<std::shared_ptr<carto::Geometry> >, geometry.GeometryVector)

#endif
