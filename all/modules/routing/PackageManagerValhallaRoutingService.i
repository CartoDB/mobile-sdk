#ifndef _PACKAGEMANAGERVALHALLAROUTINGSERVICE_I
#define _PACKAGEMANAGERVALHALLAROUTINGSERVICE_I

%module(directors="1") PackageManagerValhallaRoutingService

#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

!proxy_imports(carto::PackageManagerValhallaRoutingService, packagemanager.PackageManager, core.Variant, routing.RoutingService, routing.RoutingRequest, routing.RoutingResult, routing.RouteMatchingRequest, routing.RouteMatchingResult, datasources.TileDataSource, rastertiles.ElevationDecoder)

%{
#include "routing/PackageManagerValhallaRoutingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/Variant.i"
%import "routing/RoutingService.i"
%import "packagemanager/PackageManager.i"
%import "datasources/TileDataSource.i"
%import "rastertiles/ElevationDecoder.i"

!polymorphic_shared_ptr(carto::PackageManagerValhallaRoutingService, routing.PackageManagerValhallaRoutingService)

%std_exceptions(carto::PackageManagerValhallaRoutingService::PackageManagerValhallaRoutingService)
%std_io_exceptions(carto::PackageManagerValhallaRoutingService::matchRoute)
%std_io_exceptions(carto::PackageManagerValhallaRoutingService::calculateRoute)

%feature("director") carto::PackageManagerValhallaRoutingService;

%include "routing/PackageManagerValhallaRoutingService.h"

#endif

#endif
