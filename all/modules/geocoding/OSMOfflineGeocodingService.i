#ifndef _OSMOFFLINEGEOCODINGSERVICE_I
#define _OSMOFFLINEGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") OSMOfflineGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

!proxy_imports(carto::OSMOfflineGeocodingService, geocoding.GeocodingService, geocoding.GeocodingRequest, geocoding.GeocodingResult, projections.Projection)

%{
#include "geocoding/OSMOfflineGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geocoding/GeocodingService.i"
%import "geocoding/GeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::OSMOfflineGeocodingService, geocoding.OSMOfflineGeocodingService)

%attribute(carto::OSMOfflineGeocodingService, bool, Autocomplete, isAutocomplete, setAutocomplete)
%attributestring(carto::OSMOfflineGeocodingService, std::string, Language, getLanguage, setLanguage)
%std_io_exceptions(carto::OSMOfflineGeocodingService::OSMOfflineGeocodingService)
%std_io_exceptions(carto::OSMOfflineGeocodingService::calculateAddresses)

%feature("director") carto::OSMOfflineGeocodingService;

%include "geocoding/OSMOfflineGeocodingService.h"

#endif

#endif
