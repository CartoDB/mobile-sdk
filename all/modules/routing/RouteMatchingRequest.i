#ifndef _ROUTEMATCHINGREQUEST_I
#define _ROUTEMATCHINGREQUEST_I

#pragma SWIG nowarn=325

%module RouteMatchingRequest

#ifdef _CARTO_ROUTING_SUPPORT

!proxy_imports(carto::RouteMatchingRequest, core.MapPos, core.MapPosVector, core.Variant, projections.Projection)

%{
#include "routing/RouteMatchingRequest.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_map.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "core/Variant.i"
%import "projections/Projection.i"

!shared_ptr(carto::RouteMatchingRequest, routing.RouteMatchingRequest)

%attributestring(carto::RouteMatchingRequest, std::shared_ptr<carto::Projection>, Projection, getProjection)
%attributeval(carto::RouteMatchingRequest, std::vector<carto::MapPos>, Points, getPoints)
%attribute(carto::RouteMatchingRequest, float, Accuracy, getAccuracy)
%ignore carto::RouteMatchingRequest::getCustomParameters;
%std_exceptions(carto::RouteMatchingRequest::RouteMatchingRequest)
!standard_equals(carto::RouteMatchingRequest);
!custom_tostring(carto::RouteMatchingRequest);

%include "routing/RouteMatchingRequest.h"

#endif

#endif
