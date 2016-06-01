#ifndef _LINEGEOMETRY_I
#define _LINEGEOMETRY_I

%module LineGeometry

!proxy_imports(carto::LineGeometry, core.MapPos, core.MapPosVector, geometry.Geometry)

%{
#include "geometry/LineGeometry.h"	
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "geometry/Geometry.i"

!polymorphic_shared_ptr(carto::LineGeometry, geometry.LineGeometry)
!value_type(std::vector<std::shared_ptr<carto::LineGeometry> >, geometry.LineGeometryVector)

%attributeval(carto::LineGeometry, std::vector<carto::MapPos>, Poses, getPoses)

%include "geometry/LineGeometry.h"

!value_template(std::vector<std::shared_ptr<carto::LineGeometry> >, geometry.LineGeometryVector)

#endif
