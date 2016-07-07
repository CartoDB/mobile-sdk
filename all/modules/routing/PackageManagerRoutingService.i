#ifndef _PACKAGEMANAGERROUTINGSERVICE_I
#define _PACKAGEMANAGERROUTINGSERVICE_I

%module(directors="1") PackageManagerRoutingService

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

!proxy_imports(carto::PackageManagerRoutingService, packagemanager.PackageManager, routing.RoutingService, routing.RoutingRequest, routing.RoutingResult)

%{
#include "routing/PackageManagerRoutingService.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "routing/RoutingService.i"
%import "packagemanager/PackageManager.i"

!polymorphic_shared_ptr(carto::PackageManagerRoutingService, routing.PackageManagerRoutingService)

%std_exceptions(carto::PackageManagerRoutingService::PackageManagerRoutingService)
%std_exceptions(carto::PackageManagerRoutingService::calculateRoute)

%feature("director") carto::PackageManagerRoutingService;

%include "routing/PackageManagerRoutingService.h"

#endif

#endif
