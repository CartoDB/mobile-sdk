#ifndef _TOMTOMONLINEGEOCODINGSERVICE_I
#define _TOMTOMONLINEGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") TomTomOnlineGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT)

!proxy_imports(carto::TomTomOnlineGeocodingService, geocoding.GeocodingService, geocoding.GeocodingRequest, geocoding.GeocodingResult, projections.Projection)

%{
#include "geocoding/TomTomOnlineGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "geocoding/GeocodingService.i"
%import "geocoding/GeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::TomTomOnlineGeocodingService, geocoding.TomTomOnlineGeocodingService)

%attributestring(carto::TomTomOnlineGeocodingService, std::string, CustomServiceURL, getCustomServiceURL, setCustomServiceURL)
%std_io_exceptions(carto::TomTomOnlineGeocodingService::calculateAddresses)

%feature("director") carto::TomTomOnlineGeocodingService;

%include "geocoding/TomTomOnlineGeocodingService.h"

#endif

#endif
