#ifndef _VALHALLAOFFLINEROUTINGSERVICE_I
#define _VALHALLAOFFLINEROUTINGSERVICE_I

%module(directors="1") ValhallaOfflineRoutingService

#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

!proxy_imports(carto::ValhallaOfflineRoutingService, core.Variant, routing.RoutingService, routing.RoutingRequest, routing.RoutingResult, routing.RouteMatchingRequest, routing.RouteMatchingResult, datasources.TileDataSource, rastertiles.ElevationDecoder)

%{
#include "routing/ValhallaOfflineRoutingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/Variant.i"
%import "routing/RoutingService.i"
%import "datasources/TileDataSource.i"
%import "rastertiles/ElevationDecoder.i"

!polymorphic_shared_ptr(carto::ValhallaOfflineRoutingService, routing.ValhallaOfflineRoutingService)

%std_io_exceptions(carto::ValhallaOfflineRoutingService::ValhallaOfflineRoutingService)
%std_io_exceptions(carto::ValhallaOfflineRoutingService::matchRoute)
%std_io_exceptions(carto::ValhallaOfflineRoutingService::calculateRoute)

%feature("director") carto::ValhallaOfflineRoutingService;

%include "routing/ValhallaOfflineRoutingService.h"

#endif

#endif
