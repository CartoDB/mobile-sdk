#ifndef _ELEVATIONDECODER_I
#define _ELEVATIONDECODER_I

%module ElevationDecoder

!proxy_imports(carto::ElevationDecoder, graphics.Color, core.MapPos, core.MapPosVector, core.IntVector, datasources.TileDataSource)

%{
#include "rastertiles/ElevationDecoder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Color.i"
%import "core/MapPos.i"
%import "datasources/TileDataSource.i"
%import "core/IntVector.i"

!polymorphic_shared_ptr(carto::ElevationDecoder, rastertiles.ElevationDecoder)

!standard_equals(carto::ElevationDecoder);
%ignore carto::ElevationDecoder::getColorComponentCoefficients;
%ignore carto::ElevationDecoder::getVectorTileScales;

%include "rastertiles/ElevationDecoder.h"

#endif
