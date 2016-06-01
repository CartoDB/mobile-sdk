#ifndef _ROUTINGSERVICE_I
#define _ROUTINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") RoutingService

!proxy_imports(carto::RoutingService, routing.RoutingRequest, routing.RoutingResult)

%{
#include "routing/RoutingService.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "routing/RoutingRequest.i"
%import "routing/RoutingResult.i"

!polymorphic_shared_ptr(carto::RoutingService, routing.RoutingService)

%feature("director") carto::RoutingService;

%include "routing/RoutingService.h"

#endif
