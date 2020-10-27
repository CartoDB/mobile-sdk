#ifndef _ELEVATIONDECODER_I
#define _ELEVATIONDECODER_I

%module ElevationDecoder

!proxy_imports(carto::ElevationDecoder, graphics.Color)

%{
#include "rastertiles/ElevationDecoder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Color.i"

!polymorphic_shared_ptr(carto::ElevationDecoder, rastertiles.ElevationDecoder)

!standard_equals(carto::ElevationDecoder);
%ignore carto::ElevationDecoder::getColorComponentCoefficients;
%ignore carto::ElevationDecoder::getVectorTileScales;

%include "rastertiles/ElevationDecoder.h"

#endif
