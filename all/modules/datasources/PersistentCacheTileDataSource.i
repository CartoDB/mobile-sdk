#ifndef _PERSISTENTCACHETILEDATASOURCE_I
#define _PERSISTENTCACHETILEDATASOURCE_I

%module(directors="1") PersistentCacheTileDataSource

!proxy_imports(carto::PersistentCacheTileDataSource, core.MapTile, core.StringMap, datasources.CacheTileDataSource, datasources.components.TileData)

%{
#include "datasources/PersistentCacheTileDataSource.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "datasources/CacheTileDataSource.i"

!polymorphic_shared_ptr(carto::PersistentCacheTileDataSource, datasources.PersistentCacheTileDataSource)

%attribute(carto::PersistentCacheTileDataSource, bool, CacheOnlyMode, isCacheOnlyMode, setCacheOnlyMode)
%std_exceptions(carto::PersistentCacheTileDataSource::PersistentCacheTileDataSource)

%feature("director") carto::PersistentCacheTileDataSource;

%include "datasources/PersistentCacheTileDataSource.h"

#endif
