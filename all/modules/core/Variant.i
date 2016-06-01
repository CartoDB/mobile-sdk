#ifndef _VARIANT_I
#define _VARIANT_I

%module Variant

!proxy_imports(carto::Variant, core.StringVector, core.VariantVector, core.StringVariantMap)

%{
#include <memory>
#include <vector>
#include "core/Variant.h"
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_vector.i>
%include <std_map.i>
%include <cartoswig.i>

%import "core/StringVector.i"

!value_type(carto::Variant, core.Variant)
!value_type(std::vector<carto::Variant>, core.VariantVector)
!value_type(std::map<std::string, carto::Variant>, core.StringVariantMap)

%attribute(carto::Variant, carto::VariantType::VariantType, Type, getType)
%attribute(carto::Variant, bool, Bool, getBool)
%attribute(carto::Variant, long long, Long, getLong)
%attribute(carto::Variant, double, Double, getDouble)
%attributestring(carto::Variant, std::string, String, getString)
%attribute(carto::Variant, int, ArraySize, getArraySize)
%attributeval(carto::Variant, std::vector<std::string>, ObjectKeys, getObjectKeys)
%ignore carto::Variant::toPicoJSON;
%ignore carto::Variant::FromPicoJSON;
!custom_equals(carto::Variant);
!custom_tostring(carto::Variant);

%include "core/Variant.h"

!value_template(std::vector<carto::Variant>, core.VariantVector)
!value_template(std::map<std::string, carto::Variant>, core.StringVariantMap)

#endif
