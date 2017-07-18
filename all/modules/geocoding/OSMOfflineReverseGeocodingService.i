#ifndef _OSMOFFLINEREVERSEGEOCODINGSERVICE_I
#define _OSMOFFLINEREVERSEGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") OSMOfflineReverseGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

!proxy_imports(carto::OSMOfflineReverseGeocodingService, geocoding.ReverseGeocodingService, geocoding.ReverseGeocodingRequest, geocoding.GeocodingResult, projections.Projection)

%{
#include "geocoding/OSMOfflineReverseGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geocoding/ReverseGeocodingService.i"
%import "geocoding/ReverseGeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::OSMOfflineReverseGeocodingService, geocoding.OSMOfflineReverseGeocodingService)

%std_io_exceptions(carto::OSMOfflineReverseGeocodingService::OSMOfflineReverseGeocodingService)
%std_io_exceptions(carto::OSMOfflineReverseGeocodingService::calculateAddresses)

%feature("director") carto::OSMOfflineReverseGeocodingService;

%include "geocoding/OSMOfflineReverseGeocodingService.h"

#endif

#endif
