#ifndef _MAPBOXONLINEGEOCODINGSERVICE_I
#define _MAPBOXONLINEGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") MapBoxOnlineGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT)

!proxy_imports(carto::MapBoxOnlineGeocodingService, geocoding.GeocodingService, geocoding.GeocodingRequest, geocoding.GeocodingResult, projections.Projection)

%{
#include "geocoding/MapBoxOnlineGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "geocoding/GeocodingService.i"
%import "geocoding/GeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::MapBoxOnlineGeocodingService, geocoding.MapBoxOnlineGeocodingService)

%attribute(carto::MapBoxOnlineGeocodingService, bool, Autocomplete, isAutocomplete, setAutocomplete)
%attributestring(carto::MapBoxOnlineGeocodingService, std::string, Language, getLanguage, setLanguage)
%attributestring(carto::MapBoxOnlineGeocodingService, std::string, CustomServiceURL, getCustomServiceURL, setCustomServiceURL)
%std_io_exceptions(carto::MapBoxOnlineGeocodingService::calculateAddresses)

%feature("director") carto::MapBoxOnlineGeocodingService;

%include "geocoding/MapBoxOnlineGeocodingService.h"

#endif

#endif
