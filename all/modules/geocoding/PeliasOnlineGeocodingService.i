#ifndef _PELIASONLINEGEOCODINGSERVICE_I
#define _PELIASONLINEGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") PeliasOnlineGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT)

!proxy_imports(carto::PeliasOnlineGeocodingService, geocoding.GeocodingService, geocoding.GeocodingRequest, geocoding.GeocodingResult, projections.Projection)

%{
#include "geocoding/PeliasOnlineGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geocoding/GeocodingService.i"
%import "geocoding/GeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::PeliasOnlineGeocodingService, geocoding.PeliasOnlineGeocodingService)

%attribute(carto::PeliasOnlineGeocodingService, bool, Autocomplete, isAutocomplete, setAutocomplete)
%std_io_exceptions(carto::PeliasOnlineGeocodingService::calculateAddresses)

%feature("director") carto::PeliasOnlineGeocodingService;

%include "geocoding/PeliasOnlineGeocodingService.h"

#endif

#endif
