#ifndef _UTFGRIDRASTERTILELAYER_I
#define _UTFGRIDRASTERTILELAYER_I

%module UTFGridRasterTileLayer

!proxy_imports(carto::UTFGridRasterTileLayer, layers.RasterTileLayer, datasources.TileDataSource, core.MapPos, core.StringMap)

%{
#include "layers/UTFGridRasterTileLayer.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "core/StringMap.i"
%import "datasources/TileDataSource.i"
%import "layers/RasterTileLayer.i"

!polymorphic_shared_ptr(carto::UTFGridRasterTileLayer, layers.UTFGridRasterTileLayer)

%ignore carto::UTFGridRasterTileLayer::CombinedFetchTask;

%include "layers/UTFGridRasterTileLayer.h"

#endif
