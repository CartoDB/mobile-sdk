#ifndef _VARIANTARRAYBUILDER_I
#define _VARIANTARRAYBUILDER_I

%module VariantArrayBuilder

!proxy_imports(carto::VariantArrayBuider, core.Variant)

%{
#include "core/VariantArrayBuilder.h"
#include <memory>
%}

%include <std_string.i>
%include <cartoswig.i>

%import "core/Variant.i"

!value_type(carto::VariantArrayBuilder, core.VariantArrayBuilder)

!standard_equals(carto::VariantArrayBuilder);

%include "core/VariantArrayBuilder.h"

#endif
