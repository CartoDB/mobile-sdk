#ifndef _MULTIPOINTGEOMETRY_I
#define _MULTIPOINTGEOMETRY_I

%module MultiPointGeometry

!proxy_imports(carto::MultiPointGeometry, geometry.Geometry, geometry.MultiGeometry, geometry.PointGeometry, geometry.PointGeometryVector)

%{
#include "geometry/MultiPointGeometry.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geometry/Geometry.i"
%import "geometry/PointGeometry.i"
%import "geometry/MultiGeometry.i"

!polymorphic_shared_ptr(carto::MultiPointGeometry, geometry.MultiPointGeometry)

%std_exceptions(carto::MultiPointGeometry::getGeometry)

%include "geometry/MultiPointGeometry.h"

#endif
