#ifndef _ASSETTILEDATASOURCE_I
#define _ASSETTILEDATASOURCE_I

%module(directors="1") AssetTileDataSource

!proxy_imports(carto::AssetTileDataSource, core.MapTile, core.StringMap, datasources.TileDataSource, datasources.components.TileData)

%{
#include "datasources/AssetTileDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"

!polymorphic_shared_ptr(carto::AssetTileDataSource, datasources.AssetTileDataSource)

%feature("director") carto::AssetTileDataSource;

%include "datasources/AssetTileDataSource.h"

#endif
