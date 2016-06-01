#ifndef _CARTOONLINEROUTINGSERVICE_I
#define _CARTOONLINEROUTINGSERVICE_I

%module(directors="1") CartoOnlineRoutingService

!proxy_imports(carto::CartoOnlineRoutingService, routing.RoutingService, routing.RoutingRequest, routing.RoutingResult)

%{
#include "routing/CartoOnlineRoutingService.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "routing/RoutingService.i"

!polymorphic_shared_ptr(carto::CartoOnlineRoutingService, routing.CartoOnlineRoutingService)

%feature("director") carto::CartoOnlineRoutingService;

%include "routing/CartoOnlineRoutingService.h"

#endif
