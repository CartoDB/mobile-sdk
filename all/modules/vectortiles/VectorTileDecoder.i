#ifndef _VECTORTILEDECODER_I
#define _VECTORTILEDECODER_I

#pragma SWIG nowarn=325

%module VectorTileDecoder

!proxy_imports(carto::VectorTileDecoder, core.BinaryData, graphics.Color)

%{
#include "vectortiles/VectorTileDecoder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "graphics/Color.i"

!polymorphic_shared_ptr(carto::VectorTileDecoder, vectortiles.VectorTileDecoder)

%csmethodmodifiers carto::VectorTileDecoder::MinZoom "public virtual"
%attribute(carto::VectorTileDecoder, int, MinZoom, getMinZoom)
%csmethodmodifiers carto::VectorTileDecoder::MaxZoom "public virtual"
%attribute(carto::VectorTileDecoder, int, MaxZoom, getMaxZoom)
%ignore carto::VectorTileDecoder::decodeTile;
%ignore carto::VectorTileDecoder::getBackgroundColor;
%ignore carto::VectorTileDecoder::getBackgroundPattern;
%ignore carto::VectorTileDecoder::OnChangeListener;
%ignore carto::VectorTileDecoder::registerOnChangeListener;
%ignore carto::VectorTileDecoder::unregisterOnChangeListener;
!standard_equals(carto::VectorTileDecoder);

%include "vectortiles/VectorTileDecoder.h"

#endif
