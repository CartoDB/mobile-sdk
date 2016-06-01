#ifndef _MULTIPOLYGONGEOMETRY_I
#define _MULTIPOLYGONGEOMETRY_I

%module MultiPolygonGeometry

!proxy_imports(carto::MultiPolygonGeometry, geometry.Geometry, geometry.MultiGeometry, geometry.PolygonGeometry, geometry.PolygonGeometryVector)

%{
#include "geometry/MultiPolygonGeometry.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geometry/Geometry.i"
%import "geometry/PolygonGeometry.i"
%import "geometry/MultiGeometry.i"

!polymorphic_shared_ptr(carto::MultiPolygonGeometry, geometry.MultiPolygonGeometry)

%include "geometry/MultiPolygonGeometry.h"

#endif
