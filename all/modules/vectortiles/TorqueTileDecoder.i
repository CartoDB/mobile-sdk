#ifndef _TORQUETILEDECODER_I
#define _TORQUETILEDECODER_I

%module TorqueTileDecoder

!proxy_imports(carto::TorqueTileDecoder, core.BinaryData, graphics.Color, styles.CartoCSSStyleSet, vectortiles.VectorTileDecoder)

%{
#include "styles/CartoCSSStyleSet.h"
#include "vectortiles/TorqueTileDecoder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "styles/CartoCSSStyleSet.i"
%import "vectortiles/VectorTileDecoder.i"

!polymorphic_shared_ptr(carto::TorqueTileDecoder, vectortiles.TorqueTileDecoder)

%attribute(carto::TorqueTileDecoder, int, FrameCount, getFrameCount)
%attributestring(carto::TorqueTileDecoder, std::shared_ptr<carto::CartoCSSStyleSet>, StyleSet, getStyleSet, setStyleSet)
%ignore carto::TorqueTileDecoder::decodeTile;
%ignore carto::TorqueTileDecoder::getBackgroundColor;
%ignore carto::TorqueTileDecoder::getBackgroundPattern;

%include "vectortiles/TorqueTileDecoder.h"

#endif
