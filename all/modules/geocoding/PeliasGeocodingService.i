#ifndef _PELIASGEOCODINGSERVICE_I
#define _PELIASGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") PeliasGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT)

!proxy_imports(carto::PeliasGeocodingService, geocoding.GeocodingService, geocoding.GeocodingRequest, geocoding.GeocodingResult, projections.Projection)

%{
#include "geocoding/PeliasGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geocoding/GeocodingService.i"
%import "geocoding/GeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::PeliasGeocodingService, geocoding.PeliasGeocodingService)

%attribute(carto::PeliasGeocodingService, bool, Autocomplete, isAutocomplete, setAutocomplete)
%std_io_exceptions(carto::PeliasGeocodingService::calculateAddresses)

%feature("director") carto::PeliasGeocodingService;

%include "geocoding/PeliasGeocodingService.h"

#endif

#endif
