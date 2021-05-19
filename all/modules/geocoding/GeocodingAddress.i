#ifndef _GEOCODINGADDRESS_I
#define _GEOCODINGADDRESS_I

#pragma SWIG nowarn=325

%module GeocodingAddress

!proxy_imports(carto::GeocodingAddress, core.Address, core.StringVector)

%{
#include "geocoding/GeocodingAddress.h"
%}

%include <std_string.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/StringVector.i"
%import "core/Address.i"

!value_type(carto::GeocodingAddress, geocoding.GeocodingAddress)
!custom_tostring(carto::GecodingAddress);

%include "geocoding/GeocodingAddress.h"

#endif
