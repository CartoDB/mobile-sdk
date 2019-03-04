#ifndef _GEOMETRYSIMPLIFIER_I
#define _GEOMETRYSIMPLIFIER_I

%module GeometrySimplifier

!proxy_imports(carto::GeometrySimplifier, geometry.Geometry, projections.Projection)

%{
#include "geometry/GeometrySimplifier.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geometry/Geometry.i"
%import "projections/Projection.i"

!polymorphic_shared_ptr(carto::GeometrySimplifier, geometry.GeometrySimplifier)

%ignore carto::GeometrySimplifier::simplify;
!standard_equals(carto::GeometrySimplifier);

%include "geometry/GeometrySimplifier.h"

#endif
