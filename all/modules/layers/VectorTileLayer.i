#ifndef _VECTORTILELAYER_I
#define _VECTORTILELAYER_I

%module VectorTileLayer

!proxy_imports(carto::VectorTileLayer, core.TileData, datasources.TileDataSource, layers.TileLayer, vectortiles.VectorTileDecoder)

%{
#include "core/TileData.h"
#include "datasources/TileDataSource.h"
#include "layers/VectorTileLayer.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"
%import "layers/TileLayer.i"
%import "vectortiles/VectorTileDecoder.i"

!polymorphic_shared_ptr(carto::VectorTileLayer, layers.VectorTileLayer)

%attribute(carto::VectorTileLayer, unsigned int, TileCacheCapacity, getTileCacheCapacity, setTileCacheCapacity)
%attribute(carto::VectorTileLayer, VectorTileLabelOrder::VectorTileLabelOrder, LabelOrder, getLabelOrder, setLabelOrder)
%attributestring(carto::VectorTileLayer, std::shared_ptr<carto::VectorTileDecoder>, TileDecoder, getTileDecoder)
%ignore carto::VectorTileLayer::FetchTask;
%ignore carto::VectorTileLayer::getMinZoom;
%ignore carto::VectorTileLayer::getMaxZoom;
%ignore carto::VectorTileLayer::getCullDelay;

%include "layers/VectorTileLayer.h"

#endif
