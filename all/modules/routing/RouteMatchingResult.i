#ifndef _ROUTEMATCHINGRESULT_I
#define _ROUTEMATCHINGRESULT_I

#pragma SWIG nowarn=325

%module RouteMatchingResult

#ifdef _CARTO_ROUTING_SUPPORT

!proxy_imports(carto::RouteMatchingResult, core.MapPos, core.MapPosVector, projections.Projection, routing.RouteMatchingPoint, routing.RouteMatchingEdge)

%{
#include "routing/RouteMatchingResult.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "projections/Projection.i"
%import "routing/RouteMatchingPoint.i"
%import "routing/RouteMatchingEdge.i"

!shared_ptr(carto::RouteMatchingResult, routing.RouteMatchingResult)

%attributestring(carto::RouteMatchingResult, std::shared_ptr<carto::Projection>, Projection, getProjection)
%attributeval(carto::RouteMatchingResult, std::vector<carto::MapPos>, Points, getPoints)
%attributeval(carto::RouteMatchingResult, std::vector<carto::RouteMatchingEdge>, MatchingEdges, getMatchingEdges)
%attributeval(carto::RouteMatchingResult, std::vector<carto::RouteMatchingPoint>, MatchingPoints, getMatchingPoints)
%attributestring(carto::RouteMatchingResult, std::string, RawResult, getRawResult)
%std_exceptions(carto::RouteMatchingResult::RouteMatchingResult)
!standard_equals(carto::RouteMatchingResult);
!custom_tostring(carto::RouteMatchingResult);

%include "routing/RouteMatchingResult.h"

#endif

#endif
