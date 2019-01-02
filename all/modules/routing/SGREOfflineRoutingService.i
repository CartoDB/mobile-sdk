#ifndef _SGREMOFFLINEROUTINGSERVICE_I
#define _SGREMOFFLINEROUTINGSERVICE_I

%module(directors="1") SGREOfflineRoutingService

#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

!proxy_imports(carto::SGREOfflineRoutingService, core.Variant, geometry.FeatureCollection, projections.Projection, routing.RoutingService, routing.RoutingRequest, routing.RoutingResult)

%{
#include "routing/SGREOfflineRoutingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "routing/RoutingService.i"
%import "core/Variant.i"
%import "geometry/FeatureCollection.i"
%import "projections/Projection.i"

!polymorphic_shared_ptr(carto::SGREOfflineRoutingService, routing.SGREOfflineRoutingService)

%attributestring(carto::SGREOfflineRoutingService, std::string, Profile, getProfile, setProfile)
%std_io_exceptions(carto::SGREOfflineRoutingService::SGREOfflineRoutingService)
%std_io_exceptions(carto::SGREOfflineRoutingService::calculateRoute)

%feature("director") carto::SGREOfflineRoutingService;

%include "routing/SGREOfflineRoutingService.h"

#endif

#endif
