#ifndef _COMPRESSEDCACHETILEDATASOURCE_I
#define _COMPRESSEDCACHETILEDATASOURCE_I

%module(directors="1") CompressedCacheTileDataSource

!proxy_imports(carto::CompressedCacheTileDataSource, core.MapTile, core.StringMap, datasources.CacheTileDataSource, datasources.components.TileData)

%{
#include "datasources/CompressedCacheTileDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "datasources/CacheTileDataSource.i"

!polymorphic_shared_ptr(carto::CompressedCacheTileDataSource, datasources.CompressedCacheTileDataSource)

%feature("director") carto::CompressedCacheTileDataSource;

%include "datasources/CompressedCacheTileDataSource.h"

#endif
