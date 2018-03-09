#ifndef _TOMTOMONLINEREVERSEGEOCODINGSERVICE_I
#define _TOMTOMONLINEREVERSEGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") TomTomOnlineReverseGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT)

!proxy_imports(carto::TomTomOnlineReverseGeocodingService, geocoding.ReverseGeocodingService, geocoding.ReverseGeocodingRequest, geocoding.GeocodingResult, projections.Projection)

%{
#include "geocoding/TomTomOnlineReverseGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "geocoding/ReverseGeocodingService.i"
%import "geocoding/ReverseGeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::TomTomOnlineReverseGeocodingService, geocoding.TomTomOnlineReverseGeocodingService)

%attributestring(carto::TomTomOnlineReverseGeocodingService, std::string, Language, getLanguage, setLanguage)
%attributestring(carto::TomTomOnlineReverseGeocodingService, std::string, CustomServiceURL, getCustomServiceURL, setCustomServiceURL)
%std_io_exceptions(carto::TomTomOnlineReverseGeocodingService::calculateAddresses)

%feature("director") carto::TomTomOnlineReverseGeocodingService;

%include "geocoding/TomTomOnlineReverseGeocodingService.h"

#endif

#endif
