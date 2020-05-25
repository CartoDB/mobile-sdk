#ifndef _HILLSHADERASTERTILELAYER_I
#define _HILLSHADERASTERTILELAYER_I

%module HillshadeRasterTileLayer

!proxy_imports(carto::HillshadeRasterTileLayer, datasources.TileDataSource, layers.RasterTileLayer)

%{
#include "layers/HillshadeRasterTileLayer.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"
%import "layers/RasterTileLayer.i"

!polymorphic_shared_ptr(carto::HillshadeRasterTileLayer, layers.HillshadeRasterTileLayer)

%attribute(carto::HillshadeRasterTileLayer, float, Contrast, getContrast, setContrast)
%attribute(carto::HillshadeRasterTileLayer, float, HeightScale, getHeightScale, setHeightScale)
%std_exceptions(carto::HillshadeRasterTileLayer::HillshadeRasterTileLayer)

%include "layers/HillshadeRasterTileLayer.h"

#endif
