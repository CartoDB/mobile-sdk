#ifndef _CARTOONLINERASTERTILELAYER_I
#define _CARTOONLINERASTERTILELAYER_I

%module CartoOnlineRasterTileLayer

#ifdef _CARTO_CUSTOM_BASEMAP_SUPPORT

!proxy_imports(carto::CartoOnlineRasterTileLayer, layers.RasterTileLayer)

%{
#include "layers/CartoOnlineRasterTileLayer.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "layers/RasterTileLayer.i"
%import "utils/AssetPackage.i"

!polymorphic_shared_ptr(carto::CartoOnlineRasterTileLayer, layers.CartoOnlineRasterTileLayer)

%include "layers/CartoOnlineRasterTileLayer.h"

#endif

#endif
