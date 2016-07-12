#ifndef _LAYERS_I
#define _LAYERS_I

%module Layers

!proxy_imports(carto::Layers, layers.Layer, layers.LayerVector)

%{
#include "components/Layers.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "layers/Layer.i"

!shared_ptr(carto::Layers, components.Layers)

%typemap(cscode) carto::Layers %{ public Layer this[int index] { get { return Get(index); } set { Set(index, value); } } %}

%csmethodmodifiers carto::Layers::get "private";
%csmethodmodifiers carto::Layers::set "private";

%attribute(carto::Layers, int, Count, count)
%std_exceptions(carto::Layers::get)
%std_exceptions(carto::Layers::set)
%std_exceptions(carto::Layers::insert)
%ignore carto::Layers::Layers;
!standard_equals(carto::Layers);

%include "components/Layers.h"

#endif
