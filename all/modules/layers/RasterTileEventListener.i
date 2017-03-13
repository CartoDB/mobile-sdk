#ifndef _RASTERTILEEVENTLISTENER_I
#define _RASTERTILEEVENTLISTENER_I

%module(directors="1") RasterTileEventListener

!proxy_imports(carto::RasterTileEventListener, ui.RasterTileClickInfo)

%{
#include "layers/RasterTileEventListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "ui/RasterTileClickInfo.i"

!polymorphic_shared_ptr(carto::RasterTileEventListener, layers.RasterTileEventListener)

%feature("director") carto::RasterTileEventListener;

%include "layers/RasterTileEventListener.h"

#endif
