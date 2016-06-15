#ifndef _MBVECTORTILEDECODER_I
#define _MBVECTORTILEDECODER_I

%module MBVectorTileDecoder

!proxy_imports(carto::MBVectorTileDecoder, core.BinaryData, core.StringVector, graphics.Color, vectortiles.CompiledStyleSet, vectortiles.CartoCSSStyleSet, vectortiles.VectorTileDecoder)

%{
#include "vectortiles/MBVectorTileDecoder.h"
#include "vectortiles/CompiledStyleSet.h"
#include "vectortiles/CartoCSSStyleSet.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "vectortiles/CompiledStyleSet.i"
%import "vectortiles/CartoCSSStyleSet.i"
%import "vectortiles/VectorTileDecoder.i"

!polymorphic_shared_ptr(carto::MBVectorTileDecoder, vectortiles.MBVectorTileDecoder)

%attributeval(carto::MBVectorTileDecoder, std::vector<std::string>, StyleParameters, getStyleParameters)
%attributestring(carto::MBVectorTileDecoder, std::shared_ptr<carto::CompiledStyleSet>, CompiledStyle, getCompiledStyleSet, setCompiledStyleSet)
%attributestring(carto::MBVectorTileDecoder, std::shared_ptr<carto::CartoCSSStyleSet>, CartoCSSStyle, getCartoCSSStyleSet, setCartoCSSStyleSet)
%attribute(carto::MBVectorTileDecoder, float, Buffering, getBuffering, setBuffering)
%attribute(carto::MBVectorTileDecoder, bool, CartoCSSLayerNamesIgnored, isCartoCSSLayerNamesIgnored, setCartoCSSLayerNamesIgnored)
%attributestring(carto::MBVectorTileDecoder, std::string, LayerNameOverride, getLayerNameOverride, setLayerNameOverride)
%ignore carto::MBVectorTileDecoder::decodeTile;
%ignore carto::MBVectorTileDecoder::getBackgroundColor;
%ignore carto::MBVectorTileDecoder::getBackgroundPattern;
%ignore carto::MBVectorTileDecoder::loadMapnikMap;
%ignore carto::MBVectorTileDecoder::loadCartoCSSMap;

%include "vectortiles/MBVectorTileDecoder.h"

#endif
