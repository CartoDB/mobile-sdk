#ifndef _POLYGONGEOMETRY_I
#define _POLYGONGEOMETRY_I

%module PolygonGeometry

!proxy_imports(carto::PolygonGeometry, core.MapPos, core.MapPosVector, core.MapPosVectorVector, geometry.Geometry)

%{
#include "geometry/PolygonGeometry.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "geometry/Geometry.i"

!polymorphic_shared_ptr(carto::PolygonGeometry, geometry.PolygonGeometry)
!value_type(std::vector<std::shared_ptr<carto::PolygonGeometry> >, geometry.PolygonGeometryVector)

%attributeval(carto::PolygonGeometry, std::vector<carto::MapPos>, Poses, getPoses)
%attributeval(carto::PolygonGeometry, std::vector<std::vector<carto::MapPos> >, Holes, getHoles)
%attributeval(carto::PolygonGeometry, std::vector<std::vector<carto::MapPos> >, Rings, getRings)

%include "geometry/PolygonGeometry.h"

!value_template(std::vector<std::shared_ptr<carto::PolygonGeometry> >, geometry.PolygonGeometryVector)

#endif
