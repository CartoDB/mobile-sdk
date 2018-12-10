#ifndef _MergedMBVTTileDataSource_I
#define _MergedMBVTTileDataSource_I

%module(directors="1") MergedMBVTTileDataSource

!proxy_imports(carto::MergedMBVTTileDataSource, core.MapTile, core.MapBounds, core.StringMap, datasources.TileDataSource, datasources.components.TileData)

%{
#include "datasources/MergedMBVTTileDataSource.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"

!polymorphic_shared_ptr(carto::MergedMBVTTileDataSource, datasources.MergedMBVTTileDataSource)

%std_exceptions(carto::MergedMBVTTileDataSource::MergedMBVTTileDataSource)

%feature("director") carto::MergedMBVTTileDataSource;

%include "datasources/MergedMBVTTileDataSource.h"

#endif
