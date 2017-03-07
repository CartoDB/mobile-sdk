#ifndef _PACKAGEMANAGERGEOCODINGSERVICE_I
#define _PACKAGEMANAGERGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") PackageManagerGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

!proxy_imports(carto::PackageManagerGeocodingService, geocoding.GeocodingService, geocoding.GeocodingRequest, geocoding.GeocodingResult, packagemanager.PackageManager, projections.Projection)

%{
#include "geocoding/PackageManagerGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geocoding/GeocodingService.i"
%import "packagemanager/PackageManager.i"

!polymorphic_shared_ptr(carto::PackageManagerGeocodingService, geocoding.PackageManagerGeocodingService)

%attribute(carto::PackageManagerGeocodingService, bool, Autocomplete, isAutocomplete, setAutocomplete)
%std_exceptions(carto::PackageManagerGeocodingService::PackageManagerGeocodingService)
%std_io_exceptions(carto::PackageManagerGeocodingService::calculateAddresses)

%feature("director") carto::PackageManagerGeocodingService;

%include "geocoding/PackageManagerGeocodingService.h"

#endif

#endif
