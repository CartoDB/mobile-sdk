#ifndef _MAPTILERONLINETILEDATASOURCE_I
#define _MAPTILERONLINETILEDATASOURCE_I

%module(directors="1") MapTilerOnlineTileDataSource

!proxy_imports(carto::MapTilerOnlineTileDataSource, core.MapTile, core.MapBounds, core.StringMap, datasources.TileDataSource, datasources.components.TileData)

%{
#include "datasources/MapTilerOnlineTileDataSource.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"

!polymorphic_shared_ptr(carto::MapTilerOnlineTileDataSource, datasources.MapTilerOnlineTileDataSource)

%attributestring(carto::MapTilerOnlineTileDataSource, std::string, CustomServiceURL, getCustomServiceURL, setCustomServiceURL)
%std_exceptions(carto::MapTilerOnlineTileDataSource::MapTilerOnlineTileDataSource)
%ignore carto::MapTilerOnlineTileDataSource::buildTileURL;
%ignore carto::MapTilerOnlineTileDataSource::loadConfiguration;
%ignore carto::MapTilerOnlineTileDataSource::loadOnlineTile;

%feature("director") carto::MapTilerOnlineTileDataSource;

%include "datasources/MapTilerOnlineTileDataSource.h"

#endif
