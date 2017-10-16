#ifndef _VALHALLAOFFLINEROUTINGSERVICE_I
#define _VALHALLAOFFLINEROUTINGSERVICE_I

%module(directors="1") ValhallaOfflineRoutingService

#if defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

!proxy_imports(carto::ValhallaOfflineRoutingService, routing.RoutingService, routing.RoutingRequest, routing.RoutingResult, routing.RouteMatchingRequest, routing.RouteMatchingResult)

%{
#include "routing/ValhallaOfflineRoutingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "routing/RoutingService.i"
%import "routing/RoutingRequest.i"
%import "routing/RoutingResult.i"
%import "routing/RouteMatchingRequest.i"
%import "routing/RouteMatchingResult.i"

!polymorphic_shared_ptr(carto::ValhallaOfflineRoutingService, routing.ValhallaOfflineRoutingService)

%attributestring(carto::ValhallaOfflineRoutingService, std::string, Profile, getProfile, setProfile)
%std_io_exceptions(carto::ValhallaOfflineRoutingService::ValhallaOfflineRoutingService)
%std_io_exceptions(carto::ValhallaOfflineRoutingService::matchRoute)
%std_io_exceptions(carto::ValhallaOfflineRoutingService::calculateRoute)

%feature("director") carto::ValhallaOfflineRoutingService;

%include "routing/ValhallaOfflineRoutingService.h"

#endif

#endif
