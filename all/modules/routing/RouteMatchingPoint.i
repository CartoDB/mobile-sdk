#ifndef _ROUTEMATCHINGPOINT_I
#define _ROUTEMATCHINGPOINT_I

%module RouteMatchingPoint

#ifdef _CARTO_ROUTING_SUPPORT

!proxy_imports(carto::RouteMatchingPoint, core.MapPos)

%{
#include "routing/RouteMatchingPoint.h"
%}

%include <std_vector.i>
%include <cartoswig.i>

%import "core/MapPos.i"

!enum(carto::RouteMatchingPointType::RouteMatchingPointType)
!value_type(carto::RouteMatchingPoint, routing.RouteMatchingPoint)
!value_type(std::vector<carto::RouteMatchingPoint>, routing.RouteMatchingPointVector)

%attributeval(carto::RouteMatchingPoint, carto::MapPos, Pos, getPos)
%attribute(carto::RouteMatchingPoint, RouteMatchingPointType::RouteMatchingPointType, Type, getType)
%attribute(carto::RouteMatchingPoint, int, EdgeIndex, getEdgeIndex)
!standard_equals(carto::RouteMatchingPoint);
!custom_tostring(carto::RouteMatchingPoint);

%include "routing/RouteMatchingPoint.h"

!value_template(std::vector<carto::RouteMatchingPoint>, routing.RouteMatchingPointVector);

#endif

#endif
