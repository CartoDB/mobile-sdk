#ifndef _CACHETILEDATASOURCE_I
#define _CACHETILEDATASOURCE_I

%module(directors="1") CacheTileDataSource

!proxy_imports(carto::CacheTileDataSource, core.MapTile, core.StringMap, datasources.TileDataSource, datasources.components.TileData)

%{
#include "datasources/CacheTileDataSource.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"

!polymorphic_shared_ptr(carto::CacheTileDataSource, datasources.CacheTileDataSource)

%attribute(carto::CacheTileDataSource, std::size_t, Capacity, getCapacity, setCapacity)
%std_exceptions(carto::CacheTileDataSource::CacheTileDataSource)

%feature("director") carto::CacheTileDataSource;

%include "datasources/CacheTileDataSource.h"

#endif
