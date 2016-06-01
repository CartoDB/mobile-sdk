#ifndef _DOUGLASPEUCKERGEOMETRYSIMPLIFIER_I
#define _DOUGLASPEUCKERGEOMETRYSIMPLIFIER_I

%module DouglasPeuckerGeometrySimplifier

!proxy_imports(carto::DouglasPeuckerGeometrySimplifier, geometry.Geometry, geometry.GeometrySimplifier)

%{
#include "geometry/DouglasPeuckerGeometrySimplifier.h"
#include "core/MapPos.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "geometry/GeometrySimplifier.i"

!polymorphic_shared_ptr(carto::DouglasPeuckerGeometrySimplifier, geometry.DouglasPeuckerGeometrySimplifier)

%include "geometry/DouglasPeuckerGeometrySimplifier.h"

#endif
