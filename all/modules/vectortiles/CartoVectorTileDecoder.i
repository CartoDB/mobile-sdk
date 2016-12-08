#ifndef _CARTOVECTORTILEDECODER_I
#define _CARTOVECTORTILEDECODER_I

%module CartoVectorTileDecoder

!proxy_imports(carto::CartoVectorTileDecoder, core.BinaryData, core.stringVector, core.StringMap, graphics.Color, styles.CartoCSSStyleSet, vectortiles.VectorTileDecoder)

%{
#include "vectortiles/CartoVectorTileDecoder.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "core/StringVector.i"
%import "styles/CartoCSSStyleSet.i"
%import "vectortiles/VectorTileDecoder.i"

!polymorphic_shared_ptr(carto::CartoVectorTileDecoder, vectortiles.CartoVectorTileDecoder)

%attributeval(carto::CartoVectorTileDecoder, %arg(std::vector<std::string>), LayerIds, getLayerIds)
%std_exceptions(carto::CartoVectorTileDecoder::CartoVectorTileDecoder)
%std_exceptions(carto::CartoVectorTileDecoder::getLayerStyle)
%std_exceptions(carto::CartoVectorTileDecoder::setLayerStyle)
%ignore carto::CartoVectorTileDecoder::CartoVectorTileDecoder;
%ignore carto::CartoVectorTileDecoder::decodeFeature;
%ignore carto::CartoVectorTileDecoder::decodeTile;
%ignore carto::CartoVectorTileDecoder::getBackgroundColor;
%ignore carto::CartoVectorTileDecoder::getBackgroundPattern;
%ignore carto::CartoVectorTileDecoder::loadMapnikMap;
%ignore carto::CartoVectorTileDecoder::loadCartoCSSMap;

%include "vectortiles/CartoVectorTileDecoder.h"

#endif
