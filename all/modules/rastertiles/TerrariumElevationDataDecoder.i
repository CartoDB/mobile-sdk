#ifndef _TERRARIUMELEVATIONDATADECODER_I
#define _TERRARIUMELEVATIONDATADECODER_I

%module TerrariumElevationDataDecoder

%module(directors="1") TerrariumElevationDataDecoder
!proxy_imports(carto::TerrariumElevationDataDecoder, graphics.Color, core.MapPos, core.MapPosVector, datasources.TileDataSource, rastertiles.ElevationDecoder)

%{
#include "rastertiles/TerrariumElevationDataDecoder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Color.i"
%import "rastertiles/ElevationDecoder.i"

!polymorphic_shared_ptr(carto::TerrariumElevationDataDecoder, rastertiles.TerrariumElevationDataDecoder)
!standard_equals(carto::TerrariumElevationDataDecoder);

%feature("director") carto::TerrariumElevationDataDecoder;

%ignore carto::TerrariumElevationDataDecoder::getColorComponentCoefficients;
%ignore carto::TerrariumElevationDataDecoder::getVectorTileScales;

%include "rastertiles/TerrariumElevationDataDecoder.h"

#endif
