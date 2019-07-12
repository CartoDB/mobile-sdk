#ifndef _PACKAGEMANAGERVALHALLAROUTINGSERVICE_I
#define _PACKAGEMANAGERVALHALLAROUTINGSERVICE_I

%module(directors="1") PackageManagerValhallaRoutingService

#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

!proxy_imports(carto::PackageManagerValhallaRoutingService, packagemanager.PackageManager, routing.RoutingService, routing.RoutingRequest, routing.RoutingResult, routing.RouteMatchingRequest, routing.RouteMatchingResult)

%{
#include "routing/PackageManagerValhallaRoutingService.h"
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
%import "packagemanager/PackageManager.i"

!polymorphic_shared_ptr(carto::PackageManagerValhallaRoutingService, routing.PackageManagerValhallaRoutingService)

%attributestring(carto::PackageManagerValhallaRoutingService, std::string, Profile, getProfile, setProfile)
%std_exceptions(carto::PackageManagerValhallaRoutingService::PackageManagerValhallaRoutingService)
%std_io_exceptions(carto::PackageManagerValhallaRoutingService::matchRoute)
%std_io_exceptions(carto::PackageManagerValhallaRoutingService::calculateRoute)

%feature("director") carto::PackageManagerValhallaRoutingService;

%include "routing/PackageManagerValhallaRoutingService.h"

#endif

#endif
