#ifndef _CARTOOFFLINEROUTINGSERVICE_I
#define _CARTOOFFLINEROUTINGSERVICE_I

%module(directors="1") CartoOfflineRoutingService

!proxy_imports(carto::CartoOfflineRoutingService, routing.RoutingService, routing.RoutingRequest, routing.RoutingResult)

%{
#include "routing/CartoOfflineRoutingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "routing/RoutingService.i"

!polymorphic_shared_ptr(carto::CartoOfflineRoutingService, routing.CartoOfflineRoutingService)

%std_io_exceptions(carto::CartoOfflineRoutingService::CartoOfflineRoutingService)
%std_io_exceptions(carto::CartoOfflineRoutingService::calculateRoute)

%feature("director") carto::CartoOfflineRoutingService;

%include "routing/CartoOfflineRoutingService.h"

#endif
