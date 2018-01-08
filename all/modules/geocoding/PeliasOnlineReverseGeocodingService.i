#ifndef _PELIASONLINEREVERSEGEOCODINGSERVICE_I
#define _PELIASONLINEREVERSEGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") PeliasOnlineReverseGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT)

!proxy_imports(carto::PeliasOnlineReverseGeocodingService, geocoding.ReverseGeocodingService, geocoding.ReverseGeocodingRequest, geocoding.GeocodingResult, projections.Projection)

%{
#include "geocoding/PeliasOnlineReverseGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "geocoding/ReverseGeocodingService.i"
%import "geocoding/ReverseGeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::PeliasOnlineReverseGeocodingService, geocoding.PeliasOnlineReverseGeocodingService)

%std_io_exceptions(carto::PeliasOnlineReverseGeocodingService::calculateAddresses)

%feature("director") carto::PeliasOnlineReverseGeocodingService;

%include "geocoding/PeliasOnlineReverseGeocodingService.h"

#endif

#endif
