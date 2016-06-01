#ifndef _POINTGEOMETRY_I
#define _POINTGEOMETRY_I

%module PointGeometry

!proxy_imports(carto::PointGeometry, core.MapPos, geometry.Geometry)

%{
#include "geometry/PointGeometry.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "geometry/Geometry.i"

!polymorphic_shared_ptr(carto::PointGeometry, geometry.PointGeometry)
!value_type(std::vector<std::shared_ptr<carto::PointGeometry> >, geometry.PointGeometryVector)

%attributeval(carto::PointGeometry, carto::MapPos, Pos, getPos)

%include "geometry/PointGeometry.h"

!value_template(std::vector<std::shared_ptr<carto::PointGeometry> >, geometry.PointGeometryVector)

#endif
