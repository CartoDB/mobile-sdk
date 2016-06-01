#ifndef __CARTO_CARTOONLINEVECTORTILELAYER_I
#define __CARTO_CARTOONLINEVECTORTILELAYER_I

%module CartoOnlineVectorTileLayer

!proxy_imports(carto::CartoOnlineVectorTileLayer, layers.VectorTileLayer)

%{
#include "layers/CartoOnlineVectorTileLayer.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "layers/VectorTileLayer.i"

!polymorphic_shared_ptr(carto::CartoOnlineVectorTileLayer, layers.CartoOnlineVectorTileLayer)

%include "layers/CartoOnlineVectorTileLayer.h"

#endif
