#ifndef _MEMORYCACHETILEDATASOURCE_I
#define _MEMORYCACHETILEDATASOURCE_I

%module(directors="1") MemoryCacheTileDataSource

!proxy_imports(carto::MemoryCacheTileDataSource, core.MapTile, core.StringMap, datasources.CacheTileDataSource, datasources.components.TileData)

%{
#include "datasources/MemoryCacheTileDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "datasources/CacheTileDataSource.i"

!polymorphic_shared_ptr(carto::MemoryCacheTileDataSource, datasources.MemoryCacheTileDataSource)

%std_exceptions(carto::MemoryCacheTileDataSource::MemoryCacheTileDataSource)

%feature("director") carto::MemoryCacheTileDataSource;

%include "datasources/MemoryCacheTileDataSource.h"

#endif
