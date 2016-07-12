#ifndef _TORQUETILELAYER_I
#define _TORQUETILELAYER_I

%module TorqueTileLayer

!proxy_imports(carto::TorqueTileLayer, layers.VectorTileLayer, datasources.TileDataSource, vectortiles.TorqueTileDecoder)

%{
#include "layers/TorqueTileLayer.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "layers/VectorTileLayer.i"
%import "vectortiles/TorqueTileDecoder.i"

!polymorphic_shared_ptr(carto::TorqueTileLayer, layers.TorqueTileLayer)

%std_exceptions(carto::TorqueTileLayer::TorqueTileLayer)

%include "layers/TorqueTileLayer.h"

#endif
