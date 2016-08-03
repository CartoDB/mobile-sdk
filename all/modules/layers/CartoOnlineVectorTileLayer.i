#ifndef _CARTOONLINEVECTORTILELAYER_I
#define _CARTOONLINEVECTORTILELAYER_I

%module CartoOnlineVectorTileLayer

!proxy_imports(carto::CartoOnlineVectorTileLayer, layers.VectorTileLayer, utils.AssetPackage)

%{
#include "layers/CartoOnlineVectorTileLayer.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "layers/VectorTileLayer.i"
%import "utils/AssetPackage.i"

!polymorphic_shared_ptr(carto::CartoOnlineVectorTileLayer, layers.CartoOnlineVectorTileLayer)

%std_exceptions(carto::CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer)

%include "layers/CartoOnlineVectorTileLayer.h"

#endif
