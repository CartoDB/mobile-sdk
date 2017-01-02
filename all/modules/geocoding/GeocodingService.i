#ifndef _GEOCODINGSERVICE_I
#define _GEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") GeocodingService

#ifdef _CARTO_GEOCODING_SUPPORT

!proxy_imports(carto::GeocodingService, geocoding.GeocodingRequest, geocoding.GeocodingResult)

%{
#include "geocoding/GeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geocoding/GeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::GeocodingService, geocoding.GeocodingService)

%std_io_exceptions(carto::GeocodingService::calculateAddresses)

%feature("director") carto::GeocodingService;

%include "geocoding/GeocodingService.h"

#endif

#endif
