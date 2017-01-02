#ifndef _GEOCODINGADDRESS_I
#define _GEOCODINGADDRESS_I

#pragma SWIG nowarn=325

%module GeocodingAddress

#ifdef _CARTO_GEOCODING_SUPPORT

!proxy_imports(carto::GeocodingAddress)

%{
#include "geocoding/GeocodingAddress.h"
%}

%include <cartoswig.i>

!value_type(carto::GeocodingAddress, geocoding.GeocodingAddress)

%attributestring(carto::GeocodingAddress, std::string, Country, getCountry)
%attributestring(carto::GeocodingAddress, std::string, Region, getRegion)
%attributestring(carto::GeocodingAddress, std::string, County, getCounty)
%attributestring(carto::GeocodingAddress, std::string, Locality, getLocality)
%attributestring(carto::GeocodingAddress, std::string, Neighbourhood, getNeighbourhood)
%attributestring(carto::GeocodingAddress, std::string, Street, getStreet)
%attributestring(carto::GeocodingAddress, std::string, HouseNumber, getHouseNumber)
%attributestring(carto::GeocodingAddress, std::string, Name, getName)
%std_exceptions(carto::GeocodingAddress::GeocodingAddress)
!custom_tostring(carto::GeocodingAddress);

%include "geocoding/GeocodingAddress.h"

#endif

#endif
