#ifndef _MBTILESTILEDATASOURCE_I
#define _MBTILESTILEDATASOURCE_I

%module(directors="1") MBTilesTileDataSource

!proxy_imports(carto::MBTilesTileDataSource, core.MapTile, datasources.TileDataSource, core.TileData, core.MapBounds, core.StringMap)

%{
#include "datasources/MBTilesTileDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/MapTile.i"
%import "core/TileData.i"
%import "core/StringMap.i"
%import "datasources/TileDataSource.i"

!polymorphic_shared_ptr(carto::MBTilesTileDataSource, datasources.MBTilesTileDataSource)

%feature("director") carto::MBTilesTileDataSource;

%attributeval(carto::MBTilesTileDataSource, carto::MapBounds, DataExtent, getDataExtent)

%include "datasources/MBTilesTileDataSource.h"

#endif
