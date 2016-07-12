#ifndef _MULTILINEGEOMETRY_I
#define _MULTILINEGEOMETRY_I

%module MultiLineGeometry

!proxy_imports(carto::MultiLineGeometry, geometry.Geometry, geometry.MultiGeometry, geometry.LineGeometry, geometry.LineGeometryVector)

%{
#include "geometry/MultiLineGeometry.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geometry/Geometry.i"
%import "geometry/LineGeometry.i"
%import "geometry/MultiGeometry.i"

!polymorphic_shared_ptr(carto::MultiLineGeometry, geometry.MultiLineGeometry)

%std_exceptions(carto::MultiLineGeometry::getGeometry)

%include "geometry/MultiLineGeometry.h"

#endif
