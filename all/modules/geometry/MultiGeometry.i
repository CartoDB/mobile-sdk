#ifndef _MULTIGEOMETRY_I
#define _MULTIGEOMETRY_I

%module MultiGeometry

!proxy_imports(carto::MultiGeometry, core.MapPos, geometry.Geometry, geometry.GeometryVector)

%{
#include "geometry/MultiGeometry.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "geometry/Geometry.i"

!polymorphic_shared_ptr(carto::MultiGeometry, geometry.MultiGeometry)

%attribute(carto::MultiGeometry, int, GeometryCount, getGeometryCount)
%std_exceptions(carto::MultiGeometry::getGeometry)

%include "geometry/MultiGeometry.h"

#endif
