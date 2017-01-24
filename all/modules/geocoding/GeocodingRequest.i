#ifndef _GEOCODINGREQUEST_I
#define _GEOCODINGREQUEST_I

#pragma SWIG nowarn=325

%module GeocodingRequest

#ifdef _CARTO_GEOCODING_SUPPORT

!proxy_imports(carto::GeocodingRequest, core.MapPos, projections.Projection)

%{
#include "geocoding/GeocodingRequest.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "projections/Projection.i"

!shared_ptr(carto::GeocodingRequest, geocoding.GeocodingRequest)

%attributestring(carto::GeocodingRequest, std::string, Query, getQuery)
%attributestring(carto::GeocodingRequest, std::shared_ptr<carto::Projection>, Projection, getProjection)
%attributeval(carto::GeocodingRequest, carto::MapPos, Location, getLocation, setLocation)
%attribute(carto::GeocodingRequest, float, LocationRadius, getLocationRadius, setLocationRadius)
%std_exceptions(carto::GeocodingRequest::GeocodingRequest)
!standard_equals(carto::GeocodingRequest);
!custom_tostring(carto::GeocodingRequest);

%include "geocoding/GeocodingRequest.h"

#endif

#endif
