#ifndef _GEOCODINGREQUEST_I
#define _GEOCODINGREQUEST_I

#pragma SWIG nowarn=325

%module GeocodingRequest

#ifdef _CARTO_GEOCODING_SUPPORT

!proxy_imports(carto::GeocodingRequest)

%{
#include "geocoding/GeocodingRequest.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

!shared_ptr(carto::GeocodingRequest, geocoding.GeocodingRequest)

%attributestring(carto::GeocodingRequest, std::string, Query, getQuery)
!standard_equals(carto::GeocodingRequest);
!custom_tostring(carto::GeocodingRequest);

%include "geocoding/GeocodingRequest.h"

#endif

#endif
