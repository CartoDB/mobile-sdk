#ifndef _VALHALLAOFFLINEROUTINGSERVICE_I
#define _VALHALLAOFFLINEROUTINGSERVICE_I

%module(directors="1") ValhallaOfflineRoutingService

#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

!proxy_imports(carto::ValhallaOfflineRoutingService, routing.RoutingService, routing.RoutingRequest, routing.RoutingResult)

%{
#include "routing/ValhallaOfflineRoutingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "routing/RoutingService.i"

!polymorphic_shared_ptr(carto::ValhallaOfflineRoutingService, routing.ValhallaOfflineRoutingService)

%std_io_exceptions(carto::ValhallaOfflineRoutingService::ValhallaOfflineRoutingService)
%std_io_exceptions(carto::ValhallaOfflineRoutingService::calculateRoute)

%feature("director") carto::ValhallaOfflineRoutingService;

%include "routing/ValhallaOfflineRoutingService.h"

#endif

#endif
