#ifndef _TILEDATASOURCE_I
#define _TILEDATASOURCE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") TileDataSource

!proxy_imports(carto::TileDataSource, core.MapTile, core.StringMap, datasources.components.TileData, projections.Projection)

%{
#include "datasources/TileDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_map.i>
%include <cartoswig.i>

%import "core/MapTile.i"
%import "core/StringMap.i"
%import "datasources/components/TileData.i"
%import "projections/Projection.i"
%import "datasources/TileDataSource.i"

!polymorphic_shared_ptr(carto::TileDataSource, datasources.TileDataSource)

%attribute(carto::TileDataSource, int, MinZoom, getMinZoom)
%attribute(carto::TileDataSource, int, MaxZoom, getMaxZoom)
!attributestring_polymorphic(carto::TileDataSource, projections.Projection, Projection, getProjection)
%ignore carto::TileDataSource::OnChangeListener;
%ignore carto::TileDataSource::registerOnChangeListener;
%ignore carto::TileDataSource::unregisterOnChangeListener;

%feature("director") carto::TileDataSource;
%feature("nodirector") carto::TileDataSource::buildTagValues;

%include "datasources/TileDataSource.h"

#endif
