#ifndef _VARIANTOBJECTBUILDER_I
#define _VARIANTOBJECTBUILDER_I

%module VariantObjectBuilder

!proxy_imports(carto::VariantObjectBuider, core.Variant)

%{
#include "core/VariantObjectBuilder.h"
#include <memory>
%}

%include <std_string.i>
%include <cartoswig.i>

%import "core/Variant.i"

!value_type(carto::VariantObjectBuilder, core.VariantObjectBuilder)

!standard_equals(carto::VariantObjectBuilder);

%include "core/VariantObjectBuilder.h"

#endif
