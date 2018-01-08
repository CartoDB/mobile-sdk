#ifndef _MAPBOXONLINEREVERSEGEOCODINGSERVICE_I
#define _MAPBOXONLINEREVERSEGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") MapBoxOnlineReverseGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT)

!proxy_imports(carto::MapBoxOnlineReverseGeocodingService, geocoding.ReverseGeocodingService, geocoding.ReverseGeocodingRequest, geocoding.GeocodingResult, projections.Projection)

%{
#include "geocoding/MapBoxOnlineReverseGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "geocoding/ReverseGeocodingService.i"
%import "geocoding/ReverseGeocodingRequest.i"
%import "geocoding/GeocodingResult.i"

!polymorphic_shared_ptr(carto::MapBoxOnlineReverseGeocodingService, geocoding.MapBoxOnlineReverseGeocodingService)

%attributestring(carto::MapBoxOnlineReverseGeocodingService, std::string, Language, getLanguage, setLanguage)
%attributestring(carto::MapBoxOnlineReverseGeocodingService, std::string, CustomServiceURL, getCustomServiceURL, setCustomServiceURL)
%std_io_exceptions(carto::MapBoxOnlineReverseGeocodingService::calculateAddresses)

%feature("director") carto::MapBoxOnlineReverseGeocodingService;

%include "geocoding/MapBoxOnlineReverseGeocodingService.h"

#endif

#endif
