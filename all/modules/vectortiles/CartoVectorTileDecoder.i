#ifndef _CARTOVECTORTILEDECODER_I
#define _CARTOVECTORTILEDECODER_I

%module CartoVectorTileDecoder

!proxy_imports(carto::CartoVectorTileDecoder, core.BinaryData, core.StringVector, core.StringMap, graphics.Color, styles.CartoCSSStyleSet, styles.StringCartoCSSStyleSetMap, vectortiles.VectorTileDecoder)

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
%std_exceptions(carto::CartoVectorTileDecoder::isLayerVisible)
%std_exceptions(carto::CartoVectorTileDecoder::setLayerVisible)
%std_exceptions(carto::CartoVectorTileDecoder::getLayerStyle)
%std_exceptions(carto::CartoVectorTileDecoder::setLayerStyle)
%ignore carto::CartoVectorTileDecoder::decodeFeature;
%ignore carto::CartoVectorTileDecoder::decodeFeatures;
%ignore carto::CartoVectorTileDecoder::decodeTile;
%ignore carto::CartoVectorTileDecoder::getMapSettings;
%ignore carto::CartoVectorTileDecoder::getNutiParameters;
%ignore carto::CartoVectorTileDecoder::loadMapnikMap;
%ignore carto::CartoVectorTileDecoder::loadCartoCSSMap;

%include "vectortiles/CartoVectorTileDecoder.h"

#endif
