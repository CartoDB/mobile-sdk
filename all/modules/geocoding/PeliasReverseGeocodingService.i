#ifndef _PELIASREVERSEGEOCODINGSERVICE_I
#define _PELIASREVERSEGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") PeliasReverseGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT)

!proxy_imports(carto::PeliasReverseGeocodingService, geocoding.ReverseGeocodingService, geocoding.ReverseGeocodingRequest, geocoding.GeocodingResult, projections.Projection)

%{
#include "geocoding/PeliasReverseGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geocoding/ReverseGeocodingService.i"
%import "geocoding/ReverseGeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::PeliasReverseGeocodingService, geocoding.PeliasReverseGeocodingService)

%attribute(carto::PeliasReverseGeocodingService, float, SearchRadius, getSearchRadius, setSearchRadius)
%std_io_exceptions(carto::PeliasReverseGeocodingService::calculateAddresses)

%feature("director") carto::PeliasReverseGeocodingService;

%include "geocoding/PeliasReverseGeocodingService.h"

#endif

#endif
