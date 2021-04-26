#ifndef _HILLSHADERASTERTILELAYER_I
#define _HILLSHADERASTERTILELAYER_I

%module HillshadeRasterTileLayer

!proxy_imports(carto::HillshadeRasterTileLayer, core.MapPos, core.MapPosVector, core.DoubleVector, datasources.TileDataSource, rastertiles.ElevationDecoder, graphics.Color, layers.RasterTileLayer)

%{
#include "layers/HillshadeRasterTileLayer.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"
%import "rastertiles/ElevationDecoder.i"
%import "graphics/Color.i"
%import "layers/RasterTileLayer.i"
%import "core/DoubleVector.i"

!polymorphic_shared_ptr(carto::HillshadeRasterTileLayer, layers.HillshadeRasterTileLayer)

%attribute(carto::HillshadeRasterTileLayer, float, Contrast, getContrast, setContrast)
%attribute(carto::HillshadeRasterTileLayer, float, HeightScale, getHeightScale, setHeightScale)
%attribute(carto::HillshadeRasterTileLayer, float, IlluminationDirection, getIlluminationDirection, setIlluminationDirection)
%attribute(carto::HillshadeRasterTileLayer, bool, IlluminationMapRotationEnabled, getIlluminationMapRotationEnabled, setIlluminationMapRotationEnabled)
%attributeval(carto::HillshadeRasterTileLayer, carto::Color, ShadowColor, getShadowColor, setShadowColor)
%attributeval(carto::HillshadeRasterTileLayer, carto::Color, HighlightColor, getHighlightColor, setHighlightColor)
%std_exceptions(carto::HillshadeRasterTileLayer::HillshadeRasterTileLayer)

%include "layers/HillshadeRasterTileLayer.h"

#endif
