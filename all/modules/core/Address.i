#ifndef _ADDRESS_I
#define _ADDRESS_I

#pragma SWIG nowarn=325

%module Address

#ifdef _CARTO_GEOCODING_SUPPORT

!proxy_imports(carto::Address, core.StringVector)

%{
#include "core/Address.h"
%}

%include <std_string.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/StringVector.i"

!value_type(carto::Address, core.Address)

%attributestring(carto::Address, std::string, Country, getCountry)
%attributestring(carto::Address, std::string, Region, getRegion)
%attributestring(carto::Address, std::string, County, getCounty)
%attributestring(carto::Address, std::string, Locality, getLocality)
%attributestring(carto::Address, std::string, Neighbourhood, getNeighbourhood)
%attributestring(carto::Address, std::string, Street, getStreet)
%attributestring(carto::Address, std::string, Postcode, getPostcode)
%attributestring(carto::Address, std::string, HouseNumber, getHouseNumber)
%attributestring(carto::Address, std::string, Name, getName)
%attributeval(carto::Address, std::vector<std::string>, Categories, getCategories)
!custom_equals(carto::Address);
!custom_tostring(carto::Address);

%include "core/Address.h"

#endif

#endif
