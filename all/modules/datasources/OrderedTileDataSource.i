#ifndef _OrderedTileDataSource_I
#define _OrderedTileDataSource_I

%module(directors="1") OrderedTileDataSource

!proxy_imports(carto::OrderedTileDataSource, core.MapTile, core.MapBounds, core.StringMap, datasources.TileDataSource, datasources.components.TileData)

%{
#include "datasources/OrderedTileDataSource.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"

!polymorphic_shared_ptr(carto::OrderedTileDataSource, datasources.OrderedTileDataSource)

%std_exceptions(carto::OrderedTileDataSource::OrderedTileDataSource)

%feature("director") carto::OrderedTileDataSource;

%include "datasources/OrderedTileDataSource.h"

#endif
