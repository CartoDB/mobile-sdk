#ifndef _CARTOUIBUILDER_I
#define _CARTOUIBUILDER_I

%module(directors="1") CartoUIBuilder

!proxy_imports(carto::CartoUIBuilder, core.Variant, layers.Layer)

%{
#include "services/CartoUIBuilder.h"	
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>

%import "core/Variant.i"
%import "layers/Layer.i"

!polymorphic_shared_ptr(carto::CartoUIBuilder, services.CartoUIBuilder)

%feature("director") carto::CartoUIBuilder;

%include "services/CartoUIBuilder.h"

#endif
