#ifndef _ROUTINGSERVICE_I
#define _ROUTINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") RoutingService

#ifdef _CARTO_ROUTING_SUPPORT

!proxy_imports(carto::RoutingService, routing.RoutingRequest, routing.RoutingResult, routing.RouteMatchingRequest, routing.RouteMatchingResult)

%{
#include "routing/RoutingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "routing/RoutingRequest.i"
%import "routing/RoutingResult.i"
%import "routing/RouteMatchingRequest.i"
%import "routing/RouteMatchingResult.i"

!polymorphic_shared_ptr(carto::RoutingService, routing.RoutingService)

%attributestring(carto::RoutingService, std::string, Profile, getProfile, setProfile)
%std_exceptions(carto::RoutingService::setProfile)
%std_io_exceptions(carto::RoutingService::matchRoute)
%std_io_exceptions(carto::RoutingService::calculateRoute)

%feature("director") carto::RoutingService;

%include "routing/RoutingService.h"

#endif

#endif
