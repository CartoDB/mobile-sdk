#ifndef _ROUTINGREQUEST_I
#define _ROUTINGREQUEST_I

#pragma SWIG nowarn=325

%module RoutingRequest

!proxy_imports(carto::RoutingRequest, core.MapPos, core.MapPosVector, projections.Projection)

%{
#include "routing/RoutingRequest.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "projections/Projection.i"

!shared_ptr(carto::RoutingRequest, routing.RoutingRequest)

%attributestring(carto::RoutingRequest, std::shared_ptr<carto::Projection>, Projection, getProjection)
%attributeval(carto::RoutingRequest, std::vector<carto::MapPos>, Points, getPoints)
!standard_equals(carto::RoutingRequest);

%include "routing/RoutingRequest.h"

#endif
