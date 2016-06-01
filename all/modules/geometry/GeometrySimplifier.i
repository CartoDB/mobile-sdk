#ifndef _GEOMETRYSIMPLIFIER_I
#define _GEOMETRYSIMPLIFIER_I

%module GeometrySimplifier

!proxy_imports(carto::GeometrySimplifier, geometry.Geometry)

%{
#include "geometry/GeometrySimplifier.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geometry/Geometry.i"

!polymorphic_shared_ptr(carto::GeometrySimplifier, geometry.GeometrySimplifier)

!standard_equals(carto::GeometrySimplifier);

%include "geometry/GeometrySimplifier.h"

#endif
