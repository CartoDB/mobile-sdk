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

%include <std_string.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geocoding/GeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::GeocodingService, geocoding.GeocodingService)

%attribute(carto::GeocodingService, bool, Autocomplete, isAutocomplete, setAutocomplete)
%attributestring(carto::GeocodingService, std::string, Language, getLanguage, setLanguage)
%attribute(carto::GeocodingService, int, MaxResults, getMaxResults, setMaxResults)
%std_exceptions(carto::GeocodingService::setAutocomplete)
%std_exceptions(carto::GeocodingService::setLanguage)
%std_exceptions(carto::GeocodingService::setNumResults)
%std_io_exceptions(carto::GeocodingService::calculateAddresses)

%feature("director") carto::GeocodingService;

%include "geocoding/GeocodingService.h"

#endif

#endif
