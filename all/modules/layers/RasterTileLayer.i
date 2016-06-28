#ifndef _RASTERTILELAYER_I
#define _RASTERTILELAYER_I

%module RasterTileLayer

!proxy_imports(carto::RasterTileLayer, datasources.TileDataSource, layers.TileLayer)

%{
#include "layers/RasterTileLayer.h"
#include "layers/TileLoadListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"
%import "layers/TileLayer.i"

!polymorphic_shared_ptr(carto::RasterTileLayer, layers.RasterTileLayer)

%attribute(carto::RasterTileLayer, std::size_t, TextureCacheCapacity, getTextureCacheCapacity, setTextureCacheCapacity)
%ignore carto::RasterTileLayer::FetchTask;
%ignore carto::RasterTileLayer::getMinZoom;
%ignore carto::RasterTileLayer::getMaxZoom;

%include "layers/RasterTileLayer.h"

#endif
