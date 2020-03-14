#ifndef _REVERSEGEOCODINGSERVICE_I
#define _REVERSEGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") ReverseGeocodingService

#ifdef _CARTO_GEOCODING_SUPPORT

!proxy_imports(carto::ReverseGeocodingService, geocoding.ReverseGeocodingRequest, geocoding.GeocodingResult)

%{
#include "geocoding/ReverseGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geocoding/ReverseGeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::ReverseGeocodingService, geocoding.ReverseGeocodingService)

%attributestring(carto::ReverseGeocodingService, std::string, Language, getLanguage, setLanguage)
%std_exceptions(carto::ReverseGeocodingService::setLanguage)
%std_io_exceptions(carto::ReverseGeocodingService::calculateAddresses)

%feature("director") carto::ReverseGeocodingService;

%include "geocoding/ReverseGeocodingService.h"

#endif

#endif
