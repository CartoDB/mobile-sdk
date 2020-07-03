#ifndef _MAPBOXELEVATIONDATADECODER_I
#define _MAPBOXELEVATIONDATADECODER_I

%module MapBoxElevationDataDecoder

%module(directors="1") MapBoxElevationDataDecoder
!proxy_imports(carto::MapBoxElevationDataDecoder, graphics.Color, core.MapPos, core.MapPosVector, datasources.TileDataSource, rastertiles.ElevationDecoder)

%{
#include "rastertiles/MapBoxElevationDataDecoder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Color.i"
%import "rastertiles/ElevationDecoder.i"

!polymorphic_shared_ptr(carto::MapBoxElevationDataDecoder, rastertiles.MapBoxElevationDataDecoder)
!standard_equals(carto::MapBoxElevationDataDecoder);

%feature("director") carto::MapBoxElevationDataDecoder;

%ignore carto::MapBoxElevationDataDecoder::getColorComponentCoefficients;
%ignore carto::MapBoxElevationDataDecoder::getVectorTileScales;

%include "rastertiles/MapBoxElevationDataDecoder.h"

#endif
