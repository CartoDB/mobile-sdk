#ifndef _VALHALLAONLINEROUTINGSERVICE_I
#define _VALHALLAONLINEROUTINGSERVICE_I

%module(directors="1") ValhallaOnlineRoutingService

#if defined(_CARTO_VALHALLA_ROUTING_SUPPORT)

!proxy_imports(carto::ValhallaOnlineRoutingService, routing.RoutingService, routing.RoutingRequest, routing.RoutingResult)

%{
#include "routing/ValhallaOnlineRoutingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "routing/RoutingService.i"

!polymorphic_shared_ptr(carto::ValhallaOnlineRoutingService, routing.ValhallaOnlineRoutingService)

%attributestring(carto::ValhallaOnlineRoutingService, std::string, Profile, getProfile, setProfile)
%std_io_exceptions(carto::ValhallaOnlineRoutingService::calculateRoute)

%feature("director") carto::ValhallaOnlineRoutingService;

%include "routing/ValhallaOnlineRoutingService.h"

#endif

#endif
