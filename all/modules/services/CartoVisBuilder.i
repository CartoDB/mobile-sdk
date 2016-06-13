#ifndef _CARTOVISBUILDER_I
#define _CARTOVISBUILDER_I

%module(directors="1") CartoVisBuilder

!proxy_imports(carto::CartoVisBuilder, core.MapPos, core.MapBounds, core.Variant, layers.Layer)

%{
#include "services/CartoVisBuilder.h"	
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>

%import "core/MapPos.i"
%import "core/MapBounds.i"
%import "core/Variant.i"
%import "layers/Layer.i"

!polymorphic_shared_ptr(carto::CartoVisBuilder, services.CartoVisBuilder)

%feature("director") carto::CartoVisBuilder;

%include "services/CartoVisBuilder.h"

#endif
