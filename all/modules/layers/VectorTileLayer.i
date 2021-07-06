#ifndef _VECTORTILELAYER_I
#define _VECTORTILELAYER_I

%module VectorTileLayer

!proxy_imports(carto::VectorTileLayer, datasources.TileDataSource, datasources.components.TileData, layers.TileLayer, layers.VectorTileEventListener, vectortiles.VectorTileDecoder)

%{
#include "layers/VectorTileLayer.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"
%import "layers/VectorTileEventListener.i"
%import "layers/TileLayer.i"
%import "vectortiles/VectorTileDecoder.i"

!enum(carto::VectorTileRenderOrder::VectorTileRenderOrder)
!polymorphic_shared_ptr(carto::VectorTileLayer, layers.VectorTileLayer)

%attribute(carto::VectorTileLayer, std::size_t, TileCacheCapacity, getTileCacheCapacity, setTileCacheCapacity)
%attribute(carto::VectorTileLayer, carto::VectorTileRenderOrder::VectorTileRenderOrder, LabelRenderOrder, getLabelRenderOrder, setLabelRenderOrder)
%attribute(carto::VectorTileLayer, carto::VectorTileRenderOrder::VectorTileRenderOrder, BuildingRenderOrder, getBuildingRenderOrder, setBuildingRenderOrder)
%attribute(carto::VectorTileLayer, float, ClickRadius, getClickRadius, setClickRadius)
%attribute(carto::VectorTileLayer, float, LayerBlendingSpeed, getLayerBlendingSpeed, setLayerBlendingSpeed)
%attribute(carto::VectorTileLayer, float, LabelBlendingSpeed, getLabelBlendingSpeed, setLabelBlendingSpeed)
!attributestring_polymorphic(carto::VectorTileLayer, vectortiles.VectorTileDecoder, TileDecoder, getTileDecoder)
!attributestring_polymorphic(carto::VectorTileLayer, layers.VectorTileEventListener, VectorTileEventListener, getVectorTileEventListener, setVectorTileEventListener)
%std_exceptions(carto::VectorTileLayer::VectorTileLayer)
%ignore carto::VectorTileLayer::FetchTask;
%ignore carto::VectorTileLayer::getMinZoom;
%ignore carto::VectorTileLayer::getMaxZoom;
%ignore carto::VectorTileLayer::getCullDelay;

%include "layers/VectorTileLayer.h"

#endif
