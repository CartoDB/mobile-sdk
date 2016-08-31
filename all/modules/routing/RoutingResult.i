#ifndef _ROUTINGRESULT_I
#define _ROUTINGRESULT_I

#pragma SWIG nowarn=325

%module RoutingResult

#ifdef _CARTO_ROUTING_SUPPORT

!proxy_imports(carto::RoutingResult, core.MapPos, core.MapPosVector, projections.Projection, routing.RoutingInstruction, routing.RoutingInstructionVector)

%{
#include "routing/RoutingResult.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "projections/Projection.i"
%import "routing/RoutingInstruction.i"

!shared_ptr(carto::RoutingResult, routing.RoutingResult)

%attributestring(carto::RoutingResult, std::shared_ptr<carto::Projection>, Projection, getProjection)
%attributeval(carto::RoutingResult, std::vector<carto::MapPos>, Points, getPoints)
%attributeval(carto::RoutingResult, std::vector<carto::RoutingInstruction>, Instructions, getInstructions)
%attribute(carto::RoutingResult, double, TotalDistance, getTotalDistance)
%attribute(carto::RoutingResult, double, TotalTime, getTotalTime)
%std_exceptions(carto::RoutingResult::RoutingResult)
!standard_equals(carto::RoutingResult);

%include "routing/RoutingResult.h"

#endif

#endif
