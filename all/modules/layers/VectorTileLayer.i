#ifndef _VECTORTILELAYER_I
#define _VECTORTILELAYER_I

%module VectorTileLayer

!proxy_imports(carto::VectorTileLayer, datasources.TileDataSource, datasources.components.TileData, layers.TileLayer, vectortiles.VectorTileDecoder)

%{
#include "layers/VectorTileLayer.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"
%import "layers/TileLayer.i"
%import "vectortiles/VectorTileDecoder.i"

!polymorphic_shared_ptr(carto::VectorTileLayer, layers.VectorTileLayer)

%attribute(carto::VectorTileLayer, std::size_t, TileCacheCapacity, getTileCacheCapacity, setTileCacheCapacity)
%attribute(carto::VectorTileLayer, VectorTileRenderOrder::VectorTileRenderOrder, LabelRenderOrder, getLabelRenderOrder, setLabelRenderOrder)
%attribute(carto::VectorTileLayer, VectorTileRenderOrder::VectorTileRenderOrder, BuildingRenderOrder, getBuildingRenderOrder, setBuildingRenderOrder)
%attributestring(carto::VectorTileLayer, std::shared_ptr<carto::VectorTileDecoder>, TileDecoder, getTileDecoder)
%std_exceptions(carto::VectorTileLayer::VectorTileLayer)
%ignore carto::VectorTileLayer::FetchTask;
%ignore carto::VectorTileLayer::getMinZoom;
%ignore carto::VectorTileLayer::getMaxZoom;
%ignore carto::VectorTileLayer::getCullDelay;

%include "layers/VectorTileLayer.h"

#endif
