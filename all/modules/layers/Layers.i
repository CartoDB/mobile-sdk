#ifndef _LAYERS_I
#define _LAYERS_I

%module Layers

!proxy_imports(carto::Layers, layers.Layer, layers.LayerVector)

%{
#include "layers/Layers.h"
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "layers/Layer.i"

!shared_ptr(carto::Layers, layers.Layers)

%typemap(cscode) carto::Layers %{ public Layer this[int index] { get { return Get(index); } set { Set(index, value); } } %}

%csmethodmodifiers carto::Layers::get "private";
%csmethodmodifiers carto::Layers::set "private";

%attribute(carto::Layers, int, Count, count)
%ignore carto::Layers::Layers;
!standard_equals(carto::Layers);

%include "layers/Layers.h"

#endif
