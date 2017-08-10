#ifndef _PACKAGEMANAGERREVERSEGEOCODINGSERVICE_I
#define _PACKAGEMANAGERREVERSEGEOCODINGSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") PackageManagerReverseGeocodingService

#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

!proxy_imports(carto::PackageManagerReverseGeocodingService, geocoding.ReverseGeocodingService, geocoding.ReverseGeocodingRequest, geocoding.GeocodingResult, packagemanager.PackageManager, projections.Projection)

%{
#include "geocoding/PackageManagerReverseGeocodingService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geocoding/ReverseGeocodingService.i"
%import "packagemanager/PackageManager.i"

!polymorphic_shared_ptr(carto::PackageManagerReverseGeocodingService, geocoding.PackageManagerReverseGeocodingService)

%std_exceptions(carto::PackageManagerReverseGeocodingService::PackageManagerReverseGeocodingService)
%std_io_exceptions(carto::PackageManagerReverseGeocodingService::calculateAddresses)

%feature("director") carto::PackageManagerReverseGeocodingService;

%include "geocoding/PackageManagerReverseGeocodingService.h"

#endif

#endif
