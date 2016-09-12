#ifndef _OSRMOFFLINEROUTINGSERVICE_I
#define _OSRMOFFLINEROUTINGSERVICE_I

%module(directors="1") OSRMOfflineRoutingService

#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

!proxy_imports(carto::OSRMOfflineRoutingService, routing.RoutingService, routing.RoutingRequest, routing.RoutingResult)

%{
#include "routing/OSRMOfflineRoutingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "routing/RoutingService.i"

!polymorphic_shared_ptr(carto::OSRMOfflineRoutingService, routing.OSRMOfflineRoutingService)

%std_io_exceptions(carto::OSRMOfflineRoutingService::OSRMOfflineRoutingService)
%std_io_exceptions(carto::OSRMOfflineRoutingService::calculateRoute)

%feature("director") carto::OSRMOfflineRoutingService;

%include "routing/OSRMOfflineRoutingService.h"

#endif

#endif
