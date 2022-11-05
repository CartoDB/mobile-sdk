#ifndef _PERSISTENTCACHETILEDATASOURCE_I
#define _PERSISTENTCACHETILEDATASOURCE_I

%module(directors="1") PersistentCacheTileDataSource

#ifdef _CARTO_OFFLINE_SUPPORT

!proxy_imports(carto::PersistentCacheTileDataSource, core.MapBounds, core.MapTile, core.MapBounds, core.StringMap, datasources.CacheTileDataSource, datasources.TileDownloadListener, datasources.components.TileData)

%{
#include "datasources/PersistentCacheTileDataSource.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "datasources/CacheTileDataSource.i"
%import "datasources/TileDownloadListener.i"

!polymorphic_shared_ptr(carto::PersistentCacheTileDataSource, datasources.PersistentCacheTileDataSource)

%attribute(carto::PersistentCacheTileDataSource, bool, CacheOnlyMode, isCacheOnlyMode, setCacheOnlyMode)
%attribute(carto::PersistentCacheTileDataSource, bool, Open, isOpen)
%std_exceptions(carto::PersistentCacheTileDataSource::PersistentCacheTileDataSource)
%std_exceptions(carto::PersistentCacheTileDataSource::startDownloadArea)

%feature("director") carto::PersistentCacheTileDataSource;

%include "datasources/PersistentCacheTileDataSource.h"

#endif

#endif
