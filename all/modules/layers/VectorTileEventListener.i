#ifndef _VECTORTILEEVENTLISTENER_I
#define _VECTORTILEEVENTLISTENER_I

%module(directors="1") VectorTileEventListener

!proxy_imports(carto::VectorTileEventListener, ui.VectorTileClickInfo)

%{
#include "layers/VectorTileEventListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "ui/VectorTileClickInfo.i"

!polymorphic_shared_ptr(carto::VectorTileEventListener, layers.VectorTileEventListener)

%feature("director") carto::VectorTileEventListener;

%include "layers/VectorTileEventListener.h"

#endif
